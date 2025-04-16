cmake_minimum_required(VERSION 3.15)
project(nemo_tokenizer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    add_compile_options(/utf-8)
    add_compile_options(/openmp)
else()
    add_compile_options(-finput-charset=UTF-8)
    add_compile_options(-fopenmp)
endif()

include(FetchContent)
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v2.13.6
)
FetchContent_MakeAvailable(pybind11)

find_package(Python COMPONENTS Interpreter Development REQUIRED)

set(SOURCES
    src/nemo_tokenizer.cpp
    src/binding.cpp
)

set(HEADERS
    src/nemo_tokenizer.h
    src/json.hpp
)

pybind11_add_module(nemo_tokenizer_core ${SOURCES})

target_include_directories(nemo_tokenizer_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(nemo_tokenizer_core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/nemo_tokenizer
)

if(WIN32)
    target_compile_definitions(nemo_tokenizer_core PRIVATE _WIN32_WINNT=0x0601 -mavx2 -mfma)
endif()