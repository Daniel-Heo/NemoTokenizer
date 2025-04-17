# NemoTokenizer

NemoTokenizer is a Python package for efficient tokenization processing. It wraps core functionality implemented in C++ with a Python interface, providing both performance and usability.
It's about 60% faster than blingfire.

Currently, only the Windows version has been tested.

## Features

- Support for SentencePiece and WordPiece tokenizers
- High-performance tokenization using Trie structure
- SIMD application
- OpenMP application
- Memory-efficient implementation
- Simple Python API

## Installation

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
tokens = tokenizer.tokenize(text)
print(f"Tokens: {tokens}")

# Batch tokenize
tokens_array = tokenizer.batch_tokenize(text_array)
print(f"Tokens: {tokens_array}")

# Encoding
ids = tokenizer.encode(text)
print(f"Token IDs: {ids}")

# Decoding
decoded_text = tokenizer.decode(ids)
print(f"Decoded text: {decoded_text}")

# Token -> ID conversion
token_ids = tokenizer.convert_tokens_to_ids(tokens)
print(f"Token -> ID: {token_ids}")

# ID -> Token conversion
id_tokens = tokenizer.convert_ids_to_tokens(ids)
print(f"ID -> Token: {id_tokens}")

# Token -> Text conversion
token_text = tokenizer.convert_tokens_to_text(tokens)
print(f"Token -> TEXT: {token_text}")
```

## License

MIT License
