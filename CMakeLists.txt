cmake_minimum_required(VERSION 3.20)
project(Compiler LANGUAGES CXX)

# Common compiler library
add_library(compiler_lib STATIC
    src/lexer.cpp
    src/ast.cpp
    src/parser.cpp
    src/scope.cpp
    src/semanticAnalyzer.cpp
    src/symbol.cpp
    src/symbolTable.cpp
    src/IR.cpp
    src/IRBuilder.cpp
    src/assemblyEmitter.cpp
)

target_include_directories(compiler_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(compiler_lib PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# dyc - main compiler executable
add_executable(dyc src/main.cpp)
target_link_libraries(dyc PRIVATE compiler_lib)
set_target_properties(dyc PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# compiler_test - test executable
add_executable(compiler_test test/test.cpp)
target_link_libraries(compiler_test PRIVATE compiler_lib)
set_target_properties(compiler_test PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
