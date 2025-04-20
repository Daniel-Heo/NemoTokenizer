import os
import sys
import platform
import subprocess
import shutil
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            subprocess.check_call(["cmake", "--version"])
        except OSError:
            raise RuntimeError("CMake must be installed to build the extension")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        
        package_dir = os.path.dirname(extdir)
        if not os.path.exists(package_dir):
            os.makedirs(package_dir)
        
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPYTHON_EXECUTABLE={sys.executable}"
        ]

        cfg = "Debug" if self.debug else "Release"
        build_args = ["--config", cfg]

        if platform.system() == "Windows":
            cmake_args += [f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}"]
            build_args += ["--", "/m"]
        else:
            cmake_args += [f"-DCMAKE_BUILD_TYPE={cfg}"]
            build_args += ["--", "-j4"]     

        print(f"CMake Arg: {cmake_args}")
        print(f"Build Arg: {build_args}")
        print(f"Source Dir: {ext.sourcedir}")
        print(f"Build Dir: {self.build_temp}")
        print(f"Ext dir: {extdir}")
        
        subprocess.check_call(["cmake", ext.sourcedir] + cmake_args, cwd=self.build_temp)
        subprocess.check_call(["cmake", "--build", "."] + build_args, cwd=self.build_temp)

        print("Build products:")
        if platform.system() == "Windows":
            built_file = os.path.join(self.build_temp, cfg, "nemo_tokenizer_core.pyd")
            if os.path.exists(built_file):
                print(f"build file: {built_file}")
                shutil.copy2(built_file, extdir)
                print(f"copy: {extdir}")
            else:
                print(f"Warnning: Don't find build files: {built_file}")
                for root, dirs, files in os.walk(self.build_temp):
                    for file in files:
                        if file.startswith("nemo_tokenizer_core") and (file.endswith(".pyd") or file.endswith(".dll")):
                            found_file = os.path.join(root, file)
                            print(f"find other dir: {found_file}")
                            shutil.copy2(found_file, extdir)
                            print(f"copy: {extdir}")
        else:
            for root, dirs, files in os.walk(self.build_temp):
                for file in files:
                    if file.startswith("nemo_tokenizer_core") and (file.endswith(".so") or file.endswith(".dylib")):
                        found_file = os.path.join(root, file)
                        print(f"find files: {found_file}")
                        target_file = os.path.join(extdir, file)
                        if not os.path.exists(target_file):
                            shutil.copy2(found_file, extdir)
                            print(f"copy: {extdir}")

        print(f"Extention module dir:")
        if os.path.exists(extdir):
            for file in os.listdir(extdir):
                print(f"- {file}")
        else:
            print(f"directory not found: {extdir}")
            

setup(
    name="nemo_tokenizer",
    version="0.1.0",
    author="NemoTokenizer Team",
    author_email="example@example.com",
    description="Python binding for NemoTokenizer",
    long_description=open("README.md", encoding="utf-8").read(),
    long_description_content_type="text/markdown",
    url="https://github.com/username/nemo_tokenizer",
    packages=find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires=">=3.7",
    ext_modules=[CMakeExtension("nemo_tokenizer.nemo_tokenizer_core")],
    cmdclass={"build_ext": CMakeBuild},
    install_requires=[],
    zip_safe=False,
    package_data={
        "nemo_tokenizer": ["*.so", "*.pyd", "*.dll", "*.dylib"],
    },
)