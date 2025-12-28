cmake_minimum_required(VERSION 3.20)
project(Compiler LANGUAGES CXX)

add_executable(compiler
    src/test.cpp
)

target_include_directories(compiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(compiler PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
