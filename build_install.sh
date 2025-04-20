#!/bin/bash
set -e  # 오류 발생 시 스크립트 중단

echo "===== NemoTokenizer 시스템 전역 설치 스크립트 ====="

# 필요한 디렉토리 확인
if [ ! -d "python" ]; then
    echo "Error: python 디렉토리가 없습니다."
    exit 1
fi

# 루트 권한 확인
if [ "$EUID" -ne 0 ]; then
    echo "Error: 시스템 전역 설치를 위해 루트 권한이 필요합니다."
    echo "sudo 명령으로 이 스크립트를 다시 실행해주세요."
    exit 1
fi

# Python 버전 확인
PYTHON_VERSION=$(python3 --version | cut -d' ' -f2 | cut -d'.' -f1,2)
echo "Python 버전: $PYTHON_VERSION"

# 설치 대상 디렉토리 설정
INSTALL_DIR="/usr/local/lib/python${PYTHON_VERSION}/dist-packages"
echo "설치 대상 디렉토리: $INSTALL_DIR"

# 필요한 패키지가 시스템에 설치되어 있는지 확인
echo "시스템 패키지 설치 상태 확인..."
apt-get update

REQUIRED_PKGS="python3-pybind11 python3-dev cmake build-essential"
for pkg in $REQUIRED_PKGS; do
    if ! dpkg -l | grep -q $pkg; then
        echo "패키지 설치 중: $pkg"
        apt-get install -y $pkg
    else
        echo "이미 설치됨: $pkg"
    fi
done

# 이전 빌드 정리
echo "이전 빌드 파일 정리..."
rm -rf build
rm -rf dist
rm -rf nemo_tokenizer/*.so
rm -rf nemo_tokenizer/*.pyd
rm -rf nemo_tokenizer.egg-info

# 디렉토리 생성 확인
mkdir -p nemo_tokenizer

# 빌드 실행
echo "확장 모듈 빌드 중..."
python3 setup.py build_ext --inplace

# Python 모듈 파일 복사
echo "Python 모듈 파일 복사 중..."
cp python/tokenizer.py nemo_tokenizer/
cp python/__init__.py nemo_tokenizer/

# 대상 디렉토리 생성 확인
mkdir -p $INSTALL_DIR/nemo_tokenizer

# 수동으로 패키지 설치
echo "패키지 수동 설치 중..."
cp -r nemo_tokenizer/* $INSTALL_DIR/nemo_tokenizer/

# .egg-info 디렉토리 생성
EGGINFO_DIR="$INSTALL_DIR/nemo_tokenizer-0.1.0.egg-info"
mkdir -p $EGGINFO_DIR

# 설치 기록 파일 생성
cat > $EGGINFO_DIR/PKG-INFO << EOF
Metadata-Version: 2.1
Name: nemo_tokenizer
Version: 0.1.0
Summary: Python binding for NemoTokenizer
Author: NemoTokenizer Team
Author-email: example@example.com
License: MIT
Platform: UNKNOWN
Classifier: Programming Language :: Python :: 3
Classifier: License :: OSI Approved :: MIT License
Classifier: Operating System :: OS Independent
EOF

# __pycache__ 디렉토리 권한 설정
find $INSTALL_DIR/nemo_tokenizer -type d -exec chmod 755 {} \;
find $INSTALL_DIR/nemo_tokenizer -type f -exec chmod 644 {} \;

echo "===== 빌드 및 설치 완료 ====="
echo "설치된 Python: $(which python3)"

# 설치 확인
if python3 -c "import nemo_tokenizer; print(f'설치된 패키지 버전: {nemo_tokenizer.__version__ if hasattr(nemo_tokenizer, \"__version__\") else \"버전 정보 없음\"}')"; then
    echo "패키지가 성공적으로 설치되었습니다."
else
    echo "패키지 설치를 확인하는 중 오류가 발생했습니다."
    exit 1
fi

echo "Note: NemoTokenizer가 시스템 전역에 설치되었습니다."