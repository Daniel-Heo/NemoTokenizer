"""
NemoTokenizer 사용 예제
"""

from nemo_tokenizer import NemoTokenizer


def main():
    # 토크나이저 JSON 파일 경로
    tokenizer_file = "tokenizer.json"
    
    # 토크나이저 생성
    tokenizer = NemoTokenizer(tokenizer_file)
    
    # 테스트할 문장
    text = "안녕하세요 세계! 이것은 NemoTokenizer 테스트입니다."
    print(f"원본 텍스트: {text}")
    
    # 토큰화
    tokens = tokenizer.tokenize(text)
    print(f"토큰: {tokens}")
    
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


if __name__ == "__main__":
    main()