#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>

#include "token.hpp"

class Lexer {
    private:
        std::string source;
        size_t pos;
        int32_t line;
        int32_t column;

        char current_char();
        char peek_char();
        void advance();
        void skip_whitespace();
        void skip_comment();

        TokenData read_number();
        TokenData read_string();
        TokenData read_char();
        TokenData read_identifier();

        std::unordered_map<std::string, Token> keywords;

    public:
        Lexer(const std::string& src);
        TokenData next_token();
        std::vector<TokenData> tokenize();

};