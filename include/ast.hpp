#pragma once
#include "visitor.hpp"
#include "token.hpp"
#include <vector>

class ASTNode {
    public:
        virtual void accept(Visitor* v) = 0;
        virtual ~ASTNode() {}
};

class Expr : public ASTNode { };

class Stmt : public ASTNode { };

class LiteralExpr : public Expr {
    private:
        Token tokenType;
        std::string value; 
    public:
        LiteralExpr(Token tokenType, const std::string& value);
        void accept(Visitor* v) override;
};

class VariableExpr : public Expr {
    private:
        std::string name;
    public:
        VariableExpr(const std::string& name);
        void accept(Visitor* v) override;
};

class BinaryExpr : public Expr {
    private:
        Expr* left;
        Expr* right;
        Token op;
    public:
        BinaryExpr(Expr* left, Token op, Expr* right);
        void accept(Visitor* v) override;
};

class UnaryExpr : public Expr {
    private:
        Expr* operand;
        Token op;
    public:
        UnaryExpr(Token op, Expr* operand);
        void accept(Visitor* v) override;
};

class CallExpr : public Expr {
    private:
        std::string functionName;
        std::vector<Expr*> arguments;
    public:
        CallExpr(const std::string& functionName, const std::vector<Expr*>& arguments);
        void accept(Visitor* v) override;
};

class ArrayAccessExpr : public Expr {
    private:
        Expr* arrayName;
        Expr* index;
    public:
        ArrayAccessExpr(Expr* arrayName, Expr* index);
        void accept(Visitor* v) override;
};

class VarDeclStmt : public Stmt {
    private:
        Token type;
        std::string name;
        Expr* initializer;
    public:
        VarDeclStmt(Token type, const std::string& name, Expr* initializer);
        void accept(Visitor* v) override;
};

class AssignStmt : public Stmt {
    private:
        Expr* target;
        Expr* value;
    public:
        AssignStmt(Expr* target, Expr* value);
        void accept(Visitor* v) override;
};

class IfStmt : public Stmt {
    private:
        Expr* condition;
        Stmt* thenBranch;
        Stmt* elseBranch;
    public:
        IfStmt(Expr* condition, Stmt* thenBranch, Stmt* elseBranch);
        void accept(Visitor* v) override;
};

class WhileStmt : public Stmt {
    private:
        Expr* condition;
        Stmt* body;
    public:
        WhileStmt(Expr* condition, Stmt* body);
        void accept(Visitor* v) override;
};

class ForStmt : public Stmt {
    private:
        Stmt* initializer;
        Expr* condition;
        Expr* increment;
        Stmt* body;
    public:
        ForStmt(Stmt* initializer, Expr* condition, Expr* increment, Stmt* body);
        void accept(Visitor* v) override;
};

class ReturnStmt : public Stmt {
    private:
        Expr* returnValue;
    public:
        ReturnStmt(Expr* returnValue);
        void accept(Visitor* v) override;
};

class SendStmt : public Stmt {
    private:
        std::string targetName;
        Expr* sendValue;
    public:
        SendStmt(const std::string& targetName, Expr* sendValue);
        void accept(Visitor* v) override;
};

class RecvStmt : public Stmt {
    private:
        std::string variableName;
        std::string srcFunction;
    public:
        RecvStmt(const std::string& variableName, const std::string& srcFunction);
        void accept(Visitor* v) override;
};

class BlockStmt : public Stmt {
    private:
        std::vector<Stmt*> statements;
    public:
        BlockStmt(const std::vector<Stmt*>& statements);
        void accept(Visitor* v) override;
};

class ExprStmt : public Stmt {
    private:
        Expr* expression;
    public:
        ExprStmt(Expr* expression);
        void accept(Visitor* v) override;
};

struct Parameter {
    Token type;
    std::string name;
};

class FunctionDecl : public ASTNode {
    private:
        std::string name;
        std::vector<Parameter> parameters;
        Token returnType;
        Stmt* body;
    public:
        FunctionDecl(const std::string& name, const std::vector<Parameter>& parameters, Token returnType, Stmt* body);
        void accept(Visitor* v) override;
};

class Program : public ASTNode {
    private:
        std::vector<FunctionDecl*> functions;
    public:
        Program(const std::vector<FunctionDecl*>& functions);
        void accept(Visitor* v) override;
};