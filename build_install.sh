#!/bin/bash

echo "===== NemoTokenizer build and install script ====="

# source venv/bin/activate

echo "install python packages..."
pip install pybind11 cmake setuptools wheel

echo "clean old build..."
rm -rf build
rm -rf dist
rm -rf nemo_tokenizer/*.so
rm -rf nemo_tokenizer/*.pyd
rm -rf nemo_tokenizer.egg-info

echo "Building..."
python setup.py build_ext --inplace

cp python\tokenizer.py nemo_tokenizer
cp python\__init__.py nemo_tokenizer

echo "Install nemo_tokenizer..."
pip install -e .

echo "===== 빌드 및 설치 완료 ====="