#pragma once
#include <vector>
#include <cstdint>
#include "visitor.hpp"
#include "token.hpp"

class ASTNode {
    protected:
        int32_t line;
        int32_t column;
    public:
        ASTNode(int32_t line, int32_t column);
        int32_t get_line() const;
        int32_t get_column() const;
        virtual void accept(Visitor* v) = 0;
        virtual ~ASTNode() {}
};

class Expr : public ASTNode { 
    private:
        Token exprType;
    public:
        Expr(int32_t line, int32_t column);
        Token getType() const;
        void setType(Token type);
};

class Stmt : public ASTNode {
    public:
        Stmt(int32_t line, int32_t column);
};

class LiteralExpr : public Expr {
    private:
        Token tokenType;
        std::string value; 
    public:
        LiteralExpr(Token tokenType, const std::string& value, int32_t line, int32_t column);
        void accept(Visitor* v) override;
};

class VariableExpr : public Expr {
    private:
        std::string name;
    public:
        VariableExpr(const std::string& name, int32_t line, int32_t column);
        std::string getName() const;
        void accept(Visitor* v) override;
};

class BinaryExpr : public Expr {
    private:
        Expr* left;
        Expr* right;
        Token op;
    public:
        BinaryExpr(Expr* left, Token op, Expr* right, int32_t line, int32_t column);
        Expr* getLeft() const;
        Expr* getRight() const;
        Token getOP() const;
        void accept(Visitor* v) override;
};

class UnaryExpr : public Expr {
    private:
        Expr* operand;
        Token op;
    public:
        UnaryExpr(Token op, Expr* operand, int32_t line, int32_t column);
        Expr* getOperand() const;
        Token getOP() const;
        void accept(Visitor* v) override;
};

class CallExpr : public Expr {
    private:
        std::string functionName;
        std::vector<Expr*> arguments;
    public:
        CallExpr(const std::string& functionName, const std::vector<Expr*>& arguments, int32_t line, int32_t column);
        std::string getFunctionName() const;
        std::vector<Expr*> getArguments() const;
        void accept(Visitor* v) override;
};

class ArrayAccessExpr : public Expr {
    private:
        Expr* arrayName;
        Expr* index;
    public:
        ArrayAccessExpr(Expr* arrayName, Expr* index, int32_t line, int32_t column);
        Expr* getArrayName() const;
        Expr* getIndex() const;
        void accept(Visitor* v) override;
};

class VarDeclStmt : public Stmt {
    private:
        Token type;
        std::string name;
        Expr* initializer;
    public:
        VarDeclStmt(Token type, const std::string& name, Expr* initializer, int32_t line, int32_t column);
        Token getType() const;
        std::string getName() const;
        Expr* getInitializer() const;
        void accept(Visitor* v) override;
};

class AssignStmt : public Stmt {
    private:
        Expr* target;
        Expr* value;
    public:
        AssignStmt(Expr* target, Expr* value, int32_t line, int32_t column);
        Expr* getTarget() const;
        Expr* getValue() const;
        void accept(Visitor* v) override;
};

class IfStmt : public Stmt {
    private:
        Expr* condition;
        Stmt* thenBranch;
        Stmt* elseBranch;
    public:
        IfStmt(Expr* condition, Stmt* thenBranch, Stmt* elseBranch, int32_t line, int32_t column);
        Expr* getCondition() const;
        Stmt* getThenBranch() const;
        Stmt* getElseBranch() const;
        void accept(Visitor* v) override;
};

class WhileStmt : public Stmt {
    private:
        Expr* condition;
        Stmt* body;
    public:
        WhileStmt(Expr* condition, Stmt* body, int32_t line, int32_t column);
        Expr* getCondition() const;
        Stmt* getBody() const;
        void accept(Visitor* v) override;
};

class ForStmt : public Stmt {
    private:
        Stmt* initializer;
        Expr* condition;
        Expr* increment;
        Stmt* body;
    public:
        ForStmt(Stmt* initializer, Expr* condition, Expr* increment, Stmt* body, int32_t line, int32_t column);
        Stmt* getInitializer() const;
        Expr* getCondition() const;
        Expr* getIncrement() const;
        Stmt* getBody() const;
        void accept(Visitor* v) override;
};

class ReturnStmt : public Stmt {
    private:
        Expr* returnValue;
    public:
        ReturnStmt(Expr* returnValue, int32_t line, int32_t column);
        Expr* getReturnValue() const;
        void accept(Visitor* v) override;
};

class SendStmt : public Stmt {
    private:
        std::string variableName;
        std::string targetName;
    public:
        SendStmt(const std::string& variableName, const std::string& targetName, int32_t line, int32_t column);
        std::string getVariableName() const;
        std::string getTargetName() const;
        void accept(Visitor* v) override;
};

class RecvStmt : public Stmt {
    private:
        std::string variableName;
        std::string srcFunction;
    public:
        RecvStmt(const std::string& variableName, const std::string& srcFunction, int32_t line, int32_t column);
        std::string getVariableName() const;
        std::string getSrcFunction() const;
        void accept(Visitor* v) override;
};

class BlockStmt : public Stmt {
    private:
        std::vector<Stmt*> statements;
    public:
        BlockStmt(const std::vector<Stmt*>& statements, int32_t line, int32_t column);
        std::vector<Stmt*> getStatements() const;
        void accept(Visitor* v) override;
};

class ExprStmt : public Stmt {
    private:
        Expr* expression;
    public:
        ExprStmt(Expr* expression, int32_t line, int32_t column);
        Expr* getExpression() const;
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
        FunctionDecl(const std::string& name, const std::vector<Parameter>& parameters, Token returnType, Stmt* body, int32_t line, int32_t column);
        std::string getName() const;
        std::vector<Parameter> getParameters() const;
        Token getReturnType() const;
        Stmt* getBody() const;
        void accept(Visitor* v) override;
};

class Program : public ASTNode {
    private:
        std::vector<FunctionDecl*> functions;
    public:
        Program(const std::vector<FunctionDecl*>& functions, int32_t line, int32_t column);
        std::vector<FunctionDecl*> getFunctions() const;
        void accept(Visitor* v) override;
};