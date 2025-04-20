cmake_minimum_required(VERSION 3.15)
project(nemo_tokenizer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# SIMD Support
if(MSVC)
    add_compile_options(/arch:AVX2)
else()
    add_compile_options(-mavx2 -mfma)
endif()

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-finput-charset=UTF-8)
endif()

include(FetchContent)
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v2.13.6
)
FetchContent_MakeAvailable(pybind11)

include(FetchContent)
FetchContent_Declare(
    xsimd
    GIT_REPOSITORY https://github.com/xtensor-stack/xsimd.git
    GIT_TAG        13.2.0  # You can replace this with the specific version you want
)
FetchContent_MakeAvailable(xsimd)

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

target_include_directories(nemo_tokenizer_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${xsimd_SOURCE_DIR}/include)

if(OPENMP_FOUND)
    target_link_libraries(nemo_tokenizer_core PRIVATE OpenMP::OpenMP_CXX)
endif()

set_target_properties(nemo_tokenizer_core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/nemo_tokenizer
)

if(WIN32)
    target_compile_definitions(nemo_tokenizer_core PRIVATE _WIN32_WINNT=0x0601)
endif(WIN32)

# Debug Msg
message(STATUS "C Compiler flag: ${CMAKE_C_FLAGS}")
message(STATUS "CXX Compiler flag: ${CMAKE_CXX_FLAGS}")
message(STATUS"SIMD Activate:AVX2,FMA")