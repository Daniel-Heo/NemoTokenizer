# NemoTokenizer

NemoTokenizer는 효율적인 토큰화 처리를 위한 Python 패키지입니다. C++로 구현된 코어 기능을 Python 인터페이스로 감싸 성능과 사용성을 모두 갖추었습니다.

## 특징

- SentencePiece 및 WordPiece 토크나이저 지원
- Trie 구조를 사용한 고성능 토큰화
- 메모리 효율적인 구현
- 간편한 Python API

## 설치 방법

### 요구사항

- Python 3.6 이상
- C++ 컴파일러 (C++14 지원)
- CMake 3.12 이상

### pip를 이용한 설치 ( 미지원 )

```bash
pip install nemo_tokenizer
```

### 소스에서 설치
주의 ) xsimd include를 src 디렉토리에 넣어주셔야합니다. src/xsimd/xsimd.hpp 파일이 존재하도록.

```bash
git clone https://github.com/Daniel-Heo/NemoTokenizer.git
cd NemoTokenizer
pip install -e .
```

## 사용 예시

```python
from nemo_tokenizer import NemoTokenizer

# 토크나이저 JSON 파일 경로
tokenizer_file = "tokenizer.json"

# 토크나이저 생성
tokenizer = NemoTokenizer(tokenizer_file)

# 테스트할 문장
text = "안녕하세요 세계! 이것은 NemoTokenizer 테스트입니다."
text_array = [text]*2
print(f"원본 텍스트: {text}")

# 토큰화
tokens = tokenizer.tokenize(text)
print(f"토큰: {tokens}")

# Batch 토큰화
tokens_array = tokenizer.batch_tokenize(text_array)
print(f"토큰: {tokens_array}")

# 인코딩
ids = tokenizer.encode(text)
print(f"토큰 ID: {ids}")

# 디코딩
decoded_text = tokenizer.decode(ids)
print(f"디코딩된 텍스트: {decoded_text}")

# 토큰 -> ID 변환
token_ids = tokenizer.convert_tokens_to_ids(tokens)
print(f"토큰 -> ID: {token_ids}")

# ID -> 토큰 변환
id_tokens = tokenizer.convert_ids_to_tokens(ids)
print(f"ID -> 토큰: {id_tokens}")

# 토큰 -> Text 변환
token_text = tokenizer.convert_tokens_to_text(tokens)
print(f"토큰 -> TEXT: {token_text}")
```

## 라이센스

MIT License
