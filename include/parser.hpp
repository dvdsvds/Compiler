#pragma once
#include <vector>
#include <set>
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
        Expr* parseArray(uint32_t array_size);
        Expr* parsePostfix();  
        Expr* parsePrimary(); 

        Stmt* parseStatement();
        Stmt* parseVardecl();
        Stmt* parseIfStmt();
        Stmt* parseLoopStmt();
        Stmt* parseReturnStmt();
        Stmt* parseBreakStmt();
        Stmt* parseContinueStmt();
        Stmt* parseSendStmt();
        Stmt* parseRecvStmt();
        Stmt* parseBlock();
        Stmt* parseExprStmt();
        Stmt* parsePrintStmt();

        ImportDecl* parseImport();
        FunctionDecl* parseFunction();
        
        StructDecl* parseStruct();
        Stmt* parseStructVardecl();
        Expr* parseStructLiteral();
        bool isGlobalStructVar();

        std::set<std::string> structName;

    public:
        Parser(const std::vector<TokenData>& tokens);
        bool isGlobalVar();
        Program* parse();
};