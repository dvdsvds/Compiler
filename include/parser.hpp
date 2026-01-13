#pragma once
#include <vector>
#include "token.hpp"
#include "ast.hpp"

class Parser {
    private:
        std::vector<TokenData> tokens;
        size_t curr_pos;

        int32_t getLine();
        int32_t getColumn();

        Token peek();
        Token advance();
        bool check(Token type);
        bool match(Token type);
        bool isAtEnd();
        void expect(Token type);

        Expr* parseExpression();
        Expr* parseAssignment();
        Expr* parseLogicalOr(); 
        Expr* parseLogicalAnd(); 
        Expr* parseBitwiseOr(); 
        Expr* parseBitwiseXor(); 
        Expr* parseBitwiseAnd(); 
        Expr* parseEquality(); 
        Expr* parseRelational();
        Expr* parseShift(); 
        Expr* parseAdditive();
        Expr* parseMultiplicative(); 
        Expr* parseUnary();   
        Expr* parsePostfix();  
        Expr* parsePrimary(); 

        Stmt* parseStatement();
        Stmt* parseVardecl();
        Stmt* parseIfStmt();
        Stmt* parseLoopStmt();
        Stmt* parseReturnStmt();
        Stmt* parseSendStmt();
        Stmt* parseRecvStmt();
        Stmt* parseBlock();
        Stmt* parseExprStmt();

        FunctionDecl* parseFunction();
        
    public:
        Parser(const std::vector<TokenData>& tokens);
        Program* parse();
};