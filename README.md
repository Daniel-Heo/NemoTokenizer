﻿# NemoTokenizer

NemoTokenizer is a Python package for efficient tokenization processing. It wraps core functionality implemented in C++ with a Python interface, providing both performance and usability.

Windows/Linux install has been tested.

```
long_text = "자연어 처리(Natural Language Processing)는 컴퓨터가 인간의 언어를 이해하고 처리할 수 있게 하는 인공지능의 한 분야입니다. " * 10
start_time = time.time()
for _ in range(100000):
  tokens = tokenizer(long_text, return_tensors="pt")["input_ids"]
tokenize_time = time.time() - start_time
```

|	|NemoTokenizer	|BertTokenizerFast	|
|-------|---------------|---------------|
|Elapsed time|1.8s	    |31.4s	   |

## Features

- Support for SentencePiece and WordPiece tokenizers
- High-performance tokenization using Trie structure
- SIMD application
- OpenMP application
- Memory-efficient implementation
- Simple Python API

## Installation
Window
```
build_install.bat
```
Linux
```
chmod 755 build_install.sh
build_install.sh
```

### Requirements

- Python 3.7 or higher
- C++ compiler (with C++14 support)
- CMake 3.12 or higher

### Installation via pip (Not supported)

```bash
pip install nemo_tokenizer
```

### Installation from source

```bash
git clone https://github.com/Daniel-Heo/NemoTokenizer.git
cd NemoTokenizer
build_install.bat
```

## Usage Example

```python
from nemo_tokenizer import NemoTokenizer

# Tokenizer JSON file path
tokenizer_file = "tokenizer.json"

# Create tokenizer
tokenizer = NemoTokenizer(tokenizer_file)

# Test sentence
text = "Hello world! 이것은 NemoTokenizer 테스트입니다."
text_array = [text]*2
print(f"Original text: {text}")

# Tokenize
tokens = tokenizer.tokenize(text, add_special_tokens=True)
print(f"Tokens: {tokens}")

# Batch tokenize
tokens_array = tokenizer.batch_tokenize(text_array, add_special_tokens=True)
print(f"Tokens: {tokens_array}")

# Encoding
ids = tokenizer.encode(text, add_special_tokens=True)
print(f"Token IDs: {ids}")

# Token -> ID conversion
token_ids = tokenizer.convert_tokens_to_ids(tokens, add_special_tokens=True)
print(f"Token -> ID: {token_ids}")

# Decoding
decoded_text = tokenizer.decode(ids, skip_special_tokens=True)
print(f"Decoded text: {decoded_text}")

# ID -> Token conversion
id_tokens = tokenizer.convert_ids_to_tokens(ids, skip_special_tokens=True)
print(f"ID -> Token: {id_tokens}")

# Token -> Text conversion
token_text = tokenizer.convert_tokens_to_text(tokens, skip_special_tokens=True)
print(f"Token -> TEXT: {token_text}")
```

## License

MIT License
