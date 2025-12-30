#pragma once
#include <vector>
#include "token.hpp"
#include "ast.hpp"

class Parser {
    private:
        std::vector<TokenData> tokens;
        size_t curr_pos;

        Token peek();
        Token advance();
        bool check(Token type);
        bool match(Token type);
        bool isAtEnd();
        void expect(Token type);
    public:
        Parser(const std::vector<TokenData>& tokens);
        Program* parse();
};