"""
NemoTokenizer의 Python 인터페이스
"""

import os
from typing import List, Union, Dict, Any, Optional

try:
    from .nemo_tokenizer_core import NemoTokenizerCore
except ImportError:
    raise ImportError(
        "NemoTokenizer C++ 확장 모듈을 가져올 수 없습니다. "
        "패키지를 올바르게 설치했는지 확인하세요."
    )


class NemoTokenizer:
    """
    NemoTokenizer 클래스 - SentencePiece 및 WordPiece 토큰화를 지원합니다.
    
    C++ 구현을 감싸는 Python 인터페이스를 제공합니다.
    """
    
    def __init__(self, tokenizer_file: Optional[str] = None):
        """
        NemoTokenizer 초기화
        
        Args:
            tokenizer_file: 토크나이저 JSON 파일 경로 (선택사항)
        """
        self._tokenizer = NemoTokenizerCore()
        
        if tokenizer_file is not None:
            if not os.path.exists(tokenizer_file):
                raise FileNotFoundError(f"토크나이저 파일을 찾을 수 없습니다: {tokenizer_file}")
            self.load_tokenizer(tokenizer_file)
    
    def load_tokenizer(self, tokenizer_file: str) -> None:
        """
        토크나이저 JSON 파일 로드
        
        Args:
            tokenizer_file: 토크나이저 JSON 파일 경로
        """
        if not os.path.exists(tokenizer_file):
            raise FileNotFoundError(f"토크나이저 파일을 찾을 수 없습니다: {tokenizer_file}")
        
        self._tokenizer.loadTokenizer(tokenizer_file)
    
    def batch_tokenize(self, texts: List[str], add_special_tokens: bool = True) -> List[List[str]]:
        """
        여러 텍스트를 한번에 토큰화
        
        Args:
            texts: 토큰화할 텍스트 리스트
            add_special_tokens: 특수 토큰 추가 여부
            
        Returns:
            각 텍스트에 대한 토큰 리스트의 리스트
        """
        return self._tokenizer.batch_tokenize(texts)
    
    def tokenize(self, text: str, add_special_tokens: bool = True) -> List[str]:
        """
        텍스트를 토큰으로 분리
        
        Args:
            text: 토큰화할 텍스트
            add_special_tokens: 특수 토큰 추가 여부
            
        Returns:
            토큰 리스트
        """
        return self._tokenizer.tokenize(text)
    
    def encode(self, text: str, add_special_tokens: bool = True) -> List[int]:
        """
        텍스트를 토큰 ID로 변환
        
        Args:
            text: 인코딩할 텍스트
            add_special_tokens: 특수 토큰 추가 여부
            
        Returns:
            토큰 ID 리스트
        """
        return self._tokenizer.encode(text, add_special_tokens)
    
    def decode(self, ids: List[int]) -> str:
        """
        토큰 ID를 텍스트로 변환
        
        Args:
            ids: 디코딩할 토큰 ID 리스트
            
        Returns:
            복원된 텍스트
        """
        return self._tokenizer.decode(ids)
    
    def convert_tokens_to_ids(self, tokens: List[str]) -> List[int]:
        """
        토큰을 ID로 변환
        
        Args:
            tokens: 변환할 토큰 리스트
            
        Returns:
            토큰 ID 리스트
        """
        return self._tokenizer.convert_tokens_to_ids(tokens)
    
    def convert_ids_to_tokens(self, ids: List[int]) -> List[str]:
        """
        ID를 토큰으로 변환
        
        Args:
            ids: 변환할 ID 리스트
            
        Returns:
            토큰 리스트
        """
        return self._tokenizer.convert_ids_to_tokens(ids)
    
    def convert_tokens_to_text(self, tokens: List[str]) -> str:
        """
        토큰을 원본 텍스트로 변환
        
        Args:
            tokens: 변환할 토큰 리스트
            
        Returns:
            복원된 텍스트
        """
        return self._tokenizer.convert_tokens_to_text(tokens)