@echo off
echo ===== NemoTokenizer Build and Install Script =====

:: Install required packages
echo Installing required packages...
pip install pybind11 cmake setuptools wheel
if %ERRORLEVEL% NEQ 0 (
    echo Error: Failed to install required packages.
    pause
    exit /b %ERRORLEVEL%
)

:: Clean previous builds
echo Cleaning previous builds...
if exist build rmdir /s /q build
if exist dist rmdir /s /q dist
if exist nemo_tokenizer\*.pyd del /f /q nemo_tokenizer\*.pyd
if exist nemo_tokenizer\*.so del /f /q nemo_tokenizer\*.so
if exist nemo_tokenizer.egg-info rmdir /s /q nemo_tokenizer.egg-info

:: Build package
echo Building package...
python setup.py build_ext --inplace
if %ERRORLEVEL% NEQ 0 (
    echo Error: Build failed.
    pause
    exit /b %ERRORLEVEL%
)

copy python\tokenizer.py nemo_tokenizer
copy python\__init__.py nemo_tokenizer

:: Install package
echo Installing package...
pip install -e .
if %ERRORLEVEL% NEQ 0 (
    echo Error: Installation failed.
    pause
    exit /b %ERRORLEVEL%
)

echo ===== Build and Install Completed =====
pause