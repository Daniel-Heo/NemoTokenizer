#pragma once
#ifndef NEMO_TOKENIZER_H
#define NEMO_TOKENIZER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
#include <string>
#include <regex>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <map>
#include <forward_list>
#include <list>
#include <functional>
#include <thread>
#include <iterator>
#include <omp.h>  // OpenMP 헤더 추가
#include <xsimd/xsimd.hpp>
#include "json.hpp"

// Trie 검색 방식 선택 : 압축방식 Trie(Radix Trie)가 필요하면 적용을 고려해보자.
#define TRIE_SEARCH_TYPE 1 // 0: Low memory,low speed  1: High memory,high speed 

#if defined(_WIN32) || defined(_WIN64)
#define SELECT_ANY  __declspec(selectany)
#elif defined(__linux__)
#define SELECT_ANY  __attribute__((weak))
#elif defined(__APPLE__)
#define SELECT_ANY  __attribute__((weak))
#endif

#define OPTIMIZED

using STRING_LIST = std::list<std::string>;
#define CONCAT(A, B) A.splice(A.end(),B)

using json = nlohmann::json;
using namespace std;

// 플랫폼 독립적인 비트 연산 함수
#if defined(_MSC_VER)
#include <intrin.h>

inline unsigned int CountTrailingZeros64(uint64_t x) {
    unsigned long index;
#if defined(_WIN64)
    if (_BitScanForward64(&index, x)) {
        return index;
    }
#else
    // 32비트 Windows에서는 64비트 버전이 없으므로 수동으로 처리
    if (_BitScanForward(&index, static_cast<uint32_t>(x))) {
        return index;
    }
    if (_BitScanForward(&index, static_cast<uint32_t>(x >> 32))) {
        return index + 32;
    }
#endif
    return 64;
}

#else
// GCC, Clang 등에서는 내장 함수 사용
inline unsigned int CountTrailingZeros64(uint64_t x) {
    return x ? __builtin_ctzll(x) : 64;
}
#endif

/****************************************************************
* Class Name: NemoTokenizer
* Description: SentencePiece & WordPiece 자동 선택
*              Trie 구조를 사용하여 검색 최적화 
****************************************************************/
class NemoTokenizer {
private:
    struct TrieNode {
        bool isEnd;
        int id;
#if TRIE_SEARCH_TYPE == 1 // 속도 향상을 위한 방법
        TrieNode* children[256];
#else
        std::unordered_map<char, TrieNode*> children;
#endif

        TrieNode(): isEnd(false), id(-1) { }
    };

    struct MemoryPool {
        std::vector<TrieNode> pool;
        size_t index;

        explicit MemoryPool(size_t initialSize): index(0) {
    		pool.resize(initialSize); // resize로 미리 공간 확보만 함 (초기화 없음)
		}

        TrieNode* allocate() {
            if (index >= pool.size()) {
                size_t newSize = pool.size() * 2;
                for (size_t i = pool.size(); i < newSize; ++i) {
                    pool.resize(newSize); // 확장도 resize만으로 공간 확보
                }
            }
            TrieNode* node = &pool[index++];
        
            // node가 처음 사용될 때만 명시적으로 초기화
            node->isEnd = false;
            node->id = -1;
#if TRIE_SEARCH_TYPE == 1
            memset(node->children, 0, sizeof(node->children));
#endif
        
            return node;
        }
    };

    // 멤버 변수
    MemoryPool* nodePool;
    TrieNode* root;
    std::string decoderType; // "Metaspace" 면 SentencePiece, "WordPiece" 면 WordPiece
    std::string unkToken;    // UNK 토큰
    std::string startToken;  // 시작 토큰
    std::string endToken;    // 종료 토큰
    int unkId;               // UNK 토큰 ID
    int startId;             // 시작 토큰 ID
    int endId;               // 종료 토큰 ID
    std::string subwordPrefix; // SentencePiece의 replacement 값 또는 WordPiece의 prefix 값
    
    // ID에서 토큰으로의 빠른 변환을 위한 맵
    std::unordered_map<int, std::string> idToTokenMap;
    
    // 토큰에서 ID로의 빠른 변환을 위한 맵
    std::unordered_map<std::string, int> tokenToIdMap;

    // 내부 함수
    std::pair<std::string, int> searchLastMatchedToken(const std::string& word, bool isSubword) const {
        TrieNode* current = root;
        int lastMatchedId = -1;
        int lastMatchedPos = -1;
    
        const char* ptr = word.c_str();
        for (size_t i = 0; *ptr; ++i, ++ptr) {
            unsigned char ch = static_cast<unsigned char>(*ptr);
            if (!current->children[ch]) break;
    
            current = current->children[ch];
    
            if (current->isEnd) {
                lastMatchedId = current->id;
                lastMatchedPos = i;
            }
        }
    
        if (lastMatchedId != -1) return std::make_pair(word.substr(0, lastMatchedPos + 1), lastMatchedId);
    
        return { "", -1 };
    }

public:
    NemoTokenizer(): nodePool(nullptr), root(nullptr) {} // 생성자
    ~NemoTokenizer() { delete nodePool; } // 소멸자 (메모리 관리)

    void loadTokenizer(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: tokenizer.json 파일을 열 수 없습니다.\n";
            exit(1);
        }
    
        json tokenizer;
        file >> tokenizer;
    
        // 디코더 타입 감지 (Metaspace 면 SentencePiece, WordPiece 면 WordPiece)
        if (tokenizer.contains("decoder") && tokenizer["decoder"].contains("type")) {
            decoderType = tokenizer["decoder"]["type"].get<std::string>();
        }
        else {
            std::cerr << "Error: tokenizer.json에 decoder.type이 정의되지 않음.\n";
            exit(1);
        }
    
        // `replacement` 또는 `prefix` 값 가져오기
        if (decoderType == "Metaspace" && tokenizer["decoder"].contains("replacement")) {
            subwordPrefix = tokenizer["decoder"]["replacement"].get<std::string>();
        }
        else if (decoderType == "WordPiece" && tokenizer["decoder"].contains("prefix")) {
            subwordPrefix = tokenizer["decoder"]["prefix"].get<std::string>();
        }
        else {
            subwordPrefix = (decoderType == "Metaspace") ? "?" : "##";  // 기본값 설정
        }
    
        // UNK 토큰 확인
        if (tokenizer["model"].contains("unk_token")) {
            // sentencepiece인 경우
            if( tokenizer["model"]["unk_token"].get<std::string>() == "<unk>" ) {
                for (const auto& token : tokenizer["added_tokens"]) {
                    if (token["content"] == "<unk>") {
                        unkToken = token["content"].get<std::string>();
                        unkId = token["id"].get<int>();
                    } else if (token["content"] == "<s>") {
                        startToken = token["content"].get<std::string>();
                        startId = token["id"].get<int>();
                    } else if (token["content"] == "</s>") {
                        endToken = token["content"].get<std::string>();
                        endId = token["id"].get<int>();
                    }
                }
            } else if ( tokenizer["model"]["unk_token"].get<std::string>() == "[UNK]" ) {
                for (const auto& token : tokenizer["added_tokens"]) {
                    if (token["content"] == "[UNK]") {
                        unkToken = token["content"].get<std::string>();
                        unkId = token["id"].get<int>();
                    } else if (token["content"] == "[CLS]") {
                        startToken = token["content"].get<std::string>();
                        startId = token["id"].get<int>();
                    } else if (token["content"] == "[SEP]") {
                        endToken = token["content"].get<std::string>();
                        endId = token["id"].get<int>();
                    }
                }
            } else {
                std::cerr << "Error: none type in tokenizer.json.\n";
                exit(1);
            }
        }
        else {
            std::cerr << "Error: none type in tokenizer.json.\n";
            exit(1);
        }
    
        size_t vocabSize = tokenizer["model"]["vocab"].size();
        size_t estimatedNodes = vocabSize * 3;
    
        delete nodePool;
        nodePool = new MemoryPool(estimatedNodes);
        root = nodePool->allocate();
    
        // ID -> 토큰 맵과 토큰 -> ID 맵 초기화
        idToTokenMap.clear();
        tokenToIdMap.clear();
        idToTokenMap.reserve(vocabSize);
        tokenToIdMap.reserve(vocabSize);
    
        for (auto it = tokenizer["model"]["vocab"].begin(); it != tokenizer["model"]["vocab"].end(); ++it) {
            std::string token = it.key();
            int token_id = it.value().get<int>();
    
            // 양방향 매핑 추가
            idToTokenMap[token_id] = token;
            tokenToIdMap[token] = token_id;
    
            TrieNode* current = root;
    
            for (unsigned char ch : token) {
                if (!current->children[ch]) {
                    current->children[ch] = nodePool->allocate();
                }
                current = current->children[ch];
            }
            current->isEnd = true;
            current->id = token_id;
        }
    }

    /**
     * 텍스트를 토큰으로 분리합니다.
     * @param text 토큰화할 텍스트
     * @return 토큰 리스트
     */
    std::vector<std::string> tokenize(const std::string& text) const {
        std::vector<std::string> tokens;
        tokens.reserve(text.length() / 2);
        std::vector<std::string> words = splitWords(text);

        // 임시 버퍼를 한 번만 할당하여 재사용
        std::string buffer;
        buffer.reserve(text.length());

        for (const auto& word : words) {
            // 입력 준비 (WordPiece 또는 SentencePiece에 따라 다름)
            const char* input_ptr;
            size_t input_offset = 0;
            size_t input_length;
            
            if (decoderType == "WordPiece") {
                input_ptr = word.c_str();
                input_length = word.length();
            } else {
                // SentencePiece인 경우 prefix 추가
                buffer.clear();
                buffer = subwordPrefix;
                buffer.append(word);
                input_ptr = buffer.c_str();
                input_length = buffer.length();
            }
            
            size_t position = 0;
            bool isSubword = false;

            while (position < input_length) {
                size_t remaining = input_length - position;
                int matchedId = -1;
                int matchedLen = 0;

                // Trie 순회
                TrieNode* current = root;
                for (size_t i = 0; i < remaining; ++i) {
                    unsigned char ch = static_cast<unsigned char>(input_ptr[position + i]);
                    if (!current->children[ch]) break;
                    current = current->children[ch];
                    if (current->isEnd) {
                        matchedId = current->id;
                        matchedLen = i + 1;
                    }
                }

                if (matchedId != -1) {
                    // 직접 메모리에서 토큰 생성 (복사 최소화)
                    tokens.emplace_back(input_ptr + position, matchedLen);
                    position += matchedLen;
                    isSubword = true;
                } else {
                    // UNK 토큰은 한 번만 복사
                    tokens.push_back(unkToken);

                    // UTF-8 문자 바이트 크기 계산
                    unsigned char c = input_ptr[position];
                    int byteCount = ((c & 0x80) == 0) ? 1 :
                                    ((c & 0xE0) == 0xC0) ? 2 :
                                    ((c & 0xF0) == 0xE0) ? 3 :
                                    ((c & 0xF8) == 0xF0) ? 4 : 1;

                    position += std::min(static_cast<size_t>(byteCount), remaining);
                    isSubword = true;
                }
            }
        }

        return tokens;
    }

    /**
     * 여러 텍스트를 토큰으로 분리합니다.
     * @param texts 토큰화할 텍스트 리스트
     * @return 각 텍스트에 대한 토큰 리스트의 리스트
     */
    std::vector<std::vector<std::string>> batch_tokenize(const std::vector<std::string>& texts) const {
        std::vector<std::vector<std::string>> result(texts.size());
        omp_set_num_threads(3); // 실험적으로 조정
        #pragma omp parallel for schedule(dynamic, 1)
        for (int i = 0; i < static_cast<int>(texts.size()); ++i) {
            result[i] = tokenize(texts[i]);
        }
    
        return result;
    }

    /**
     * 텍스트를 직접 토큰 ID로 변환합니다.
     * @param text 변환할 텍스트
     * @param add_special_tokens 특수 토큰(시작, 종료) 추가 여부
     * @return 토큰 ID 리스트
     */
    std::vector<int> encode(const std::string& text, bool add_special_tokens = true) const {
        std::vector<int> ids;
        ids.reserve(text.length() / 2 + (add_special_tokens ? 2 : 0)); // 평균 토큰 길이를 2로 가정하고 공간 예약
        std::vector<std::string> words = splitWords(text);
        
        if (add_special_tokens) {
            ids.push_back(startId);  // 시작 토큰 추가
        }
        
        // 임시 버퍼를 한 번만 할당하여 재사용
        std::string buffer;
        buffer.reserve(text.length());
        
        for (const auto& word : words) {
            // 입력 준비 (WordPiece 또는 SentencePiece에 따라 다름)
            const char* input_ptr;
            size_t input_length;
            
            if (decoderType == "WordPiece") {
                input_ptr = word.c_str();
                input_length = word.length();
            } else {
                // SentencePiece인 경우 prefix 추가
                buffer.clear();
                buffer = subwordPrefix;
                buffer.append(word);
                input_ptr = buffer.c_str();
                input_length = buffer.length();
            }
            
            size_t position = 0;
            bool isSubword = false;
        
            while (position < input_length) {
                int matchedId = -1;
                int matchedLen = 0;
                
                // 필요한 경우만 서브워드 접두사 처리
                if (isSubword && decoderType == "WordPiece") {
                    buffer.clear();
                    buffer = subwordPrefix;
                    buffer.append(input_ptr + position, input_length - position);
                    input_ptr = buffer.c_str();
                    matchedLen = 0; // 매치된 길이 초기화
                    
                    // Trie에서 토큰을 찾고 바로 ID를 가져옴
                    TrieNode* current = root;
                    
                    for (size_t i = 0; i < buffer.length(); ++i) {
                        unsigned char ch = static_cast<unsigned char>(buffer[i]);
                        if (!current->children[ch]) break;
                        
                        current = current->children[ch];
                        
                        if (current->isEnd) {
                            matchedId = current->id;
                            matchedLen = i + 1;
                        }
                    }
                    
                    if (matchedId != -1) {
                        ids.push_back(matchedId);
                        // 접두사 길이를 제외한 실제 진행된 위치 계산
                        position += matchedLen - subwordPrefix.length();
                    }
                } else {
                    // 일반 토큰 처리
                    TrieNode* current = root;
                    
                    for (size_t i = 0; i < input_length - position; ++i) {
                        unsigned char ch = static_cast<unsigned char>(input_ptr[position + i]);
                        if (!current->children[ch]) break;
                        
                        current = current->children[ch];
                        
                        if (current->isEnd) {
                            matchedId = current->id;
                            matchedLen = i + 1;
                        }
                    }
                    
                    if (matchedId != -1) {
                        ids.push_back(matchedId);
                        position += matchedLen;
                    }
                }
                
                // 매치되지 않은 경우 UNK 토큰 추가
                if (matchedId == -1) {
                    ids.push_back(unkId);
                    
                    // UTF-8 문자 바이트 크기 계산
                    char c = input_ptr[position];
                    int byteCount;
                    if ((c & 0x80) == 0) byteCount = 1;
                    else if ((c & 0xE0) == 0xC0) byteCount = 2;
                    else if ((c & 0xF0) == 0xE0) byteCount = 3;
                    else if ((c & 0xF8) == 0xF0) byteCount = 4;
                    else byteCount = 1;
                    
                    if ((byteCount + position) > input_length)
                        byteCount = input_length - position;
                    
                    position += byteCount;
                }
                
                isSubword = true;
            }
        }
        
        if (add_special_tokens) {
            ids.push_back(endId);  // 종료 토큰 추가
        }
        
        return ids;
    }

    /**
     * 토큰 ID 리스트를 텍스트로 변환합니다.
     * @param ids 변환할 토큰 ID 리스트
     * @return 복원된 텍스트
     */
    std::string decode(const std::vector<int>& ids) const {
        std::string result;
        result.reserve(ids.size() * 4); // 예상 텍스트 길이 미리 할당
        
        for (size_t i = 0; i < ids.size(); ++i) {
            int id = ids[i];
            
            // 특수 토큰 건너뛰기 옵션 처리
            if (id == startId || id == endId) {
                continue;
            }
            
            auto it = idToTokenMap.find(id);
            if (it != idToTokenMap.end()) {
                std::string token = it->second;
                
                // 토큰 유형에 따른 처리
                if (decoderType == "WordPiece") {
                    // WordPiece 처리
                    if (token.rfind(subwordPrefix, 0) == 0) {
                        // 서브워드(##으로 시작)는 공백 없이 이어붙임
                        result += token.substr(subwordPrefix.size());
                    } else {
                        // 일반 토큰은 앞에 공백 추가 (첫 토큰 제외)
                        if (!result.empty()) {
                            result += ' ';
                        }
                        result += token;
                    }
                } else if (decoderType == "Metaspace") {
                    // SentencePiece 처리
                    bool isWordStart = token.rfind(subwordPrefix, 0) == 0;
                    
                    if (isWordStart) {
                        // 단어 시작 토큰이면 앞에 공백 추가 (첫 토큰 제외)
                        if (!result.empty()) {
                            result += ' ';
                        }
                        result += token.substr(subwordPrefix.size());
                    } else {
                        // 단어 중간/끝 토큰이면 공백 없이 이어붙임
                        result += token;
                    }
                } else {
                    // 기타 디코더 유형 처리
                    if (!result.empty()) {
                        result += ' ';
                    }
                    result += token;
                }
            }
        }
        
        return result;
    }

    /**
     * 토큰 리스트를 ID 리스트로 변환합니다.
     * @param tokens 변환할 토큰 리스트
     * @return 토큰 ID 리스트
     */
    std::vector<int> convert_tokens_to_ids(const std::vector<std::string>& tokens) const {
        std::vector<int> ids;
        ids.reserve(tokens.size());
        
        for (const auto& token : tokens) {
            auto it = tokenToIdMap.find(token);
            if (it != tokenToIdMap.end()) {
                ids.push_back(it->second);
            } else {
                ids.push_back(unkId);
            }
        }
        
        return ids;
    }

    /**
     * ID 리스트를 토큰 리스트로 변환합니다.
     * @param ids 변환할 ID 리스트
     * @return 토큰 리스트
     */
    std::vector<std::string> convert_ids_to_tokens(const std::vector<int>& ids) const {
        std::vector<std::string> tokens;
        tokens.reserve(ids.size());
        
        for (int id : ids) {
            auto it = idToTokenMap.find(id);
            if (it != idToTokenMap.end()) {
                tokens.push_back(it->second);
            } else {
                tokens.push_back(unkToken);
            }
        }
        
        return tokens;
    }

    /**
     * 토큰 리스트를 원본 텍스트로 변환합니다.
     * @param tokens 변환할 토큰 리스트
     * @return 복원된 텍스트
     */
    std::string convert_tokens_to_text(const std::vector<std::string>& tokens) const {
        std::string result;
        result.reserve(tokens.size() * 4); // 예상 텍스트 길이 미리 할당
        
        for (size_t i = 0; i < tokens.size(); ++i) {
            const std::string& token = tokens[i];
            
            // 특수 토큰(시작/종료) 건너뛰기
            if (token == startToken || token == endToken) {
                continue;
            }
            
            // 토큰 유형에 따른 처리
            if (decoderType == "WordPiece") {
                // WordPiece 처리
                if (token.rfind(subwordPrefix, 0) == 0) {
                    // 서브워드(##으로 시작)는 공백 없이 이어붙임
                    result += token.substr(subwordPrefix.size());
                } else {
                    // 일반 토큰은 앞에 공백 추가 (첫 토큰 제외)
                    if (!result.empty()) {
                        result += ' ';
                    }
                    result += token;
                }
            } else if (decoderType == "Metaspace") {
                // SentencePiece 처리
                bool isWordStart = token.rfind(subwordPrefix, 0) == 0;
                
                if (isWordStart) {
                    // 단어 시작 토큰이면 앞에 공백 추가 (첫 토큰 제외)
                    if (!result.empty()) {
                        result += ' ';
                    }
                    result += token.substr(subwordPrefix.size());
                } else {
                    // 단어 중간/끝 토큰이면 공백 없이 이어붙임
                    result += token;
                }
            } else {
                // 기타 디코더 유형 처리
                if (!result.empty()) {
                    result += ' ';
                }
                result += token;
            }
        }
        
        return result;
    }

    /**
     * 텍스트를 공백 문자로 구분하여 단어 벡터로 반환합니다.
     * XSIMD를 사용하여 성능을 최적화한 버전입니다.
     * 
     * @param text 분리할 텍스트
     * @return 공백으로 구분된 단어들의 벡터
     */
    std::vector<std::string> splitWords(const std::string& text) const {
        std::vector<std::string> result;
        if (text.empty()) return result;
    
        result.reserve(text.length() / 3); // 예상 단어 수 확보
    
        const char* data = text.data();
        const size_t length = text.length();
        size_t word_start = 0;
        size_t i = 0;
    
        using batch_type = xsimd::batch<int8_t>;
        using batch_bool_type = xsimd::batch_bool<int8_t>;
        constexpr size_t simd_size = batch_type::size;
    
        while (i + simd_size <= length) {
            batch_type data_batch = xsimd::load_unaligned(reinterpret_cast<const int8_t*>(&data[i]));
        
            batch_bool_type is_space =
                (data_batch == batch_type(' ')) |
                (data_batch == batch_type('\t')) |
                (data_batch == batch_type('\n')) |
                (data_batch == batch_type('\r'));
        
            uint64_t mask = is_space.mask();
        
            while (mask) {
                int offset = CountTrailingZeros64(mask);
                size_t pos = i + offset;
        
                if (pos > word_start) {
                    result.emplace_back(&data[word_start], pos - word_start);
                }
        
                word_start = pos + 1;
                mask &= ~(1ULL << offset);
            }
        
            i += simd_size;
        }
    
        // 나머지 처리 (SIMD 범위 밖)
        for (; i < length; ++i) {
            if (std::isspace(static_cast<unsigned char>(data[i]))) {
                if (i > word_start) {
                    result.emplace_back(&data[word_start], i - word_start);
                }
                word_start = i + 1;
            }
        }
    
        // 마지막 단어 처리
        if (word_start < length) {
            result.emplace_back(&data[word_start], length - word_start);
        }
    
        return result;
    }
};

#endif