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

```bash
git clone https://github.com/username/nemo_tokenizer](https://github.com/Daniel-Heo/NemoTokenizer.git
cd nemo_tokenizer
pip install -e .
```

## 사용 예시

```python
from nemo_tokenizer import NemoTokenizer

# 토크나이저 생성
tokenizer = NemoTokenizer("path/to/tokenizer.json")

# 문장 토큰화
tokens = tokenizer.tokenize("안녕하세요 세계!")
print(tokens)

# 인코딩
ids = tokenizer.encode("안녕하세요 세계!")
print(ids)

# 디코딩
text = tokenizer.decode(ids)
print(text)
```

## 라이센스

MIT License
