cmake_minimum_required(VERSION 3.20)
project(Compiler LANGUAGES CXX)

add_executable(compiler
    test/test.cpp
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
    src/assembler.cpp
)

target_include_directories(compiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(compiler PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
