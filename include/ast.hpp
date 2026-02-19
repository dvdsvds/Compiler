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
        Token getTokenType() const;
        std::string getValue() const;
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

class InExpr : public Expr {
    private:
        std::string variableName;
    public:
        InExpr(const std::string& variableName, int32_t line, int32_t column);
        std::string getVariableName() const;
        void accept(Visitor* v) override;
};

class OutExpr : public Expr {
    private:
        std::string variableName;
    public:
        OutExpr(const std::string& variableName, int32_t line, int32_t column);
        std::string getVariableName() const;
        void accept(Visitor* v) override;
};

class ReferenceExpr : public Expr {
    private:
        Expr* expr;
    public:
        ReferenceExpr(Expr* expr, int32_t line, int32_t column); 
        Expr* getExpr() const;
        void accept(Visitor* v) override;
};

class DereferenceExpr : public Expr {
    private:
        Expr* expr;
    public:
        DereferenceExpr(Expr* expr, int32_t line, int32_t column);
        Expr* getExpr() const;
        void accept(Visitor* v) override;
};

class ArrayExpr : public Expr {
    private:
        std::vector<Expr*> elements;
        uint32_t array_size;
    public:
        ArrayExpr(const std::vector<Expr*>& elements, uint32_t array_size, int32_t line, int32_t column);
        std::vector<Expr*> getElements() const;
        uint32_t getArraySize() const;
        void accept(Visitor* v) override;
};

class VarDeclStmt : public Stmt {
    private:
        Token type;
        std::string name;
        std::string struct_name;
        Expr* initializer;
        bool address_taken;
        uint32_t array_size;
        bool isPointerType;
    public:
        VarDeclStmt(Token type, const std::string& name, const std::string& struct_name, Expr* initializer, uint32_t array_size, bool isPointerType, int32_t line, int32_t column);
        Token getType() const;
        std::string getName() const;
        std::string getStructName() const;
        bool isPointer() const;
        uint32_t getArraySize() const;
        Expr* getInitializer() const;
        bool isAddressTaken() const;
        void setAddressTaken(bool taken);
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

class LoopStmt : public Stmt {
    private:
        Stmt* initializer;
        Expr* condition;
        Expr* increment;
        Stmt* body;

    public:
        LoopStmt(Stmt* initializer, Expr* condition, Expr* increment, Stmt* body, int32_t line, int32_t column);
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

class BreakStmt : public Stmt {
    public:
        BreakStmt(int line, int col);
        void accept(Visitor* v) override;
};

class ContinueStmt : public Stmt {
    public:
        ContinueStmt(int line, int col);
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
        Token varType;
    public:
        RecvStmt(const std::string& variableName, const std::string& srcFunction, int32_t line, int32_t column);
        std::string getVariableName() const;
        std::string getSrcFunction() const;
        Token getVarType() const;
        void setVarType(Token type);
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

class PrintStmt : public Stmt {
    private:
        Expr* expression;
    public:
        PrintStmt(Expr* expression, int32_t line, int32_t column);
        Expr* getExpression() const;
        void accept(Visitor* v) override;
};

struct Parameter {
    Token type;
    std::string name;
    std::string structName;
};

class FunctionDecl : public ASTNode {
    private:
        std::string name;
        std::vector<Parameter> parameters;
        Token returnType;
        std::string returnStructName;
        Stmt* body;
    public:
        FunctionDecl(const std::string& name, const std::vector<Parameter>& parameters, Token returnType, const std::string& returnStructName, Stmt* body, int32_t line, int32_t column);
        std::string getName() const;
        std::string getReturnStructName() const;
        std::vector<Parameter> getParameters() const;
        Token getReturnType() const;
        Stmt* getBody() const;
        void accept(Visitor* v) override;
};

class ImportDecl : public ASTNode {
    private:
        std::string moduleName;
        std::vector<std::string> symbols;
    public:
        ImportDecl(const std::string& moduleName, const std::vector<std::string>& symbols, int32_t line, int32_t column);
        std::string getModuleName() const;
        std::vector<std::string> getSymbols() const;
        bool isSelectiveImport() const;
        void accept(Visitor* v) override;
};

struct StructMember {
    Token type;
    std::string name;
};

class StructDecl : public ASTNode {
    private:
        std::string name;
        std::vector<StructMember> members;
    public:
        StructDecl(const std::string& name, const std::vector<StructMember>& members, int32_t line, int32_t column);
        std::string getName() const;
        std::vector<StructMember> getMembers() const;
        void accept(Visitor* v) override;
};

class MemberAccessExpr : public Expr {
    private:
        Expr* object;
        std::string memberName;
        bool isArrow;
    public:
        MemberAccessExpr(Expr* object, const std::string& memberName, int32_t line, int32_t column, bool isArrow = false);
        Expr* getObject() const;
        bool isArrowAccess() const;
        std::string getMemberName() const;
        void accept(Visitor* v) override;
};

class StructLiteralExpr : public Expr {
    private:
        std::vector<Expr*> values;
    public:
        StructLiteralExpr(const std::vector<Expr*>& values, int32_t line, int32_t column);
            std::vector<Expr*> getValues() const;
        void accept(Visitor* v) override;
};

class Program : public ASTNode {
    private:
        std::vector<ImportDecl*> imports;
        std::vector<VarDeclStmt*> globals;
        std::vector<FunctionDecl*> functions;
        std::vector<StructDecl*> structs;
    public:
        Program(const std::vector<ImportDecl*>& imports, const std::vector<VarDeclStmt*>& globals, const std::vector<FunctionDecl*>& functions, const std::vector<StructDecl*>& structs, int32_t line, int32_t column);
        std::vector<ImportDecl*> getImports() const;
        std::vector<VarDeclStmt*> getGlobals() const;
        std::vector<FunctionDecl*> getFunctions() const;
        std::vector<StructDecl*> getStructs() const;
        void accept(Visitor* v) override;
};