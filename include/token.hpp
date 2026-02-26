#pragma once
#include <string>

enum class Token {
    IMPORT, 
    PRINT,
    STRUCT, STRUCT_T, PSTRUCT_T, FUNC_PTR,
    ENUM, 
    IF, ELSE, 
    LOOP, 
    IN, OUT, 
    RETURN, BREAK, CONTINUE, SWITCH, CASE, DEFAULT, 
    ASM, 
    SEND, RECV, 
    NULL_KW,
    S8, S16, S32, US8, US16, US32, BOOL, TRUE, FALSE, VOID, NULL_T,
    S8_ARRAY, S16_ARRAY, S32_ARRAY, US8_ARRAY, US16_ARRAY, US32_ARRAY, BOOL_ARRAY, 
    PS8, PS16, PS32, PUS8, PUS16, PUS32,
    IDENTIFIER, NUMBER, 
    STRING ,CHAR,
    PLUS, MINUS, STAR, SLASH, PERCENT,
    EQ, NE, LT, GT, LE, GE,
    AND, OR, NOT,
    BIT_AND, BIT_OR, BIT_XOR, BIT_NOT, SHL, SHR,
    ASSIGN, PLUS_EQ, MINUS_EQ, STAR_EQ, SLASH_EQ, PERCENT_EQ, AND_EQ, OR_EQ, XOR_EQ, SHL_EQ, SHR_EQ,
    INC, DEC,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET, SEMICOLON, COLON, COMMA, DOT,
    ARROW, RARROW, LARROW, HASH, AS,
    GLOBAL,
    EOF_TOKEN, INVALID
};

struct TokenData {
    Token type;
    std::string value;
    int line;
    int column;
};

inline std::string tokenToString(Token t) {
    switch(t) {
        case Token::IMPORT: return "IMPORT";

        case Token::STRUCT: return "STRUCT";
        case Token::ENUM: return "ENUM";
        case Token::IF: return "IF";
        case Token::ELSE: return "ELSE";
        case Token::LOOP: return "LOOP";
        case Token::IN: return "IN";
        case Token::OUT: return "OUT";
        case Token::RETURN: return "RETURN";
        case Token::BREAK: return "BREAK";
        case Token::CONTINUE: return "CONTINUE";
        case Token::SWITCH: return "SWITCH";
        case Token::CASE: return "CASE";
        case Token::DEFAULT: return "DEFAULT";
        case Token::ASM: return "ASM";
        case Token::SEND: return "SEND";
        case Token::RECV: return "RECV";
        case Token::NULL_KW: return "NULL_KW";
        
        case Token::S8: return "S8";
        case Token::S16: return "S16";
        case Token::S32: return "S32";
        case Token::US8: return "US8";
        case Token::US16: return "US16";
        case Token::US32: return "US32";
        case Token::BOOL: return "BOOL";
        case Token::TRUE: return "TRUE";
        case Token::FALSE: return "FALSE";
        case Token::VOID: return "VOID";
        case Token::NULL_T: return "NULL_T";
        
        case Token::S8_ARRAY: return "S8[]";
        case Token::S16_ARRAY: return "S16[]";
        case Token::S32_ARRAY: return "S32[]";
        case Token::US8_ARRAY: return "US8[]";
        case Token::US16_ARRAY: return "US16[]";
        case Token::US32_ARRAY: return "US32[]";
        case Token::BOOL_ARRAY: return "BOOL[]";
        
        case Token::PS8: return "PS8";
        case Token::PS16: return "PS16";
        case Token::PS32: return "PS32";
        case Token::PUS8: return "PUS8";
        case Token::PUS16: return "PUS16";
        case Token::PUS32: return "PUS32";
        
        default: return "UNKNOWN";
    }
}