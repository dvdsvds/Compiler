#include "ast.hpp"

LiteralExpr::LiteralExpr(Token tokenType, const std::string& value) : tokenType(tokenType), value(value) {}
void LiteralExpr::accept(Visitor* v) { v->visit(this); }

VariableExpr::VariableExpr(const std::string& name) : name(name) {}
std::string VariableExpr::getName() const { return name; }
void VariableExpr::accept(Visitor* v) { v->visit(this); }

BinaryExpr::BinaryExpr(Expr* left, Token op, Expr* right) : left(left), op(op), right(right) {}
void BinaryExpr::accept(Visitor* v) { v->visit(this); }

UnaryExpr::UnaryExpr(Token op, Expr* operand) : op(op), operand(operand) {}
void UnaryExpr::accept(Visitor* v) { v->visit(this); }

CallExpr::CallExpr(const std::string& functionName, const std::vector<Expr*>& arguments) : functionName(functionName), arguments(arguments) {}
void CallExpr::accept(Visitor* v) { v->visit(this); }

ArrayAccessExpr::ArrayAccessExpr(Expr* arrayName, Expr* index) : arrayName(arrayName), index(index) {}
void ArrayAccessExpr::accept(Visitor* v) { v->visit(this); }

VarDeclStmt::VarDeclStmt(Token type, const std::string& name, Expr* initializer) : type(type), name(name), initializer(initializer) {}
void VarDeclStmt::accept(Visitor* v) { v->visit(this); }

AssignStmt::AssignStmt(Expr* target, Expr* value) : target(target), value(value) {}
void AssignStmt::accept(Visitor* v) { v->visit(this); }

IfStmt::IfStmt(Expr* condition, Stmt* thenBranch, Stmt* elseBranch) : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}
void IfStmt::accept(Visitor* v) { v->visit(this); }

WhileStmt::WhileStmt(Expr* condition, Stmt* body) : condition(condition), body(body) {}
void WhileStmt::accept(Visitor* v) { v->visit(this); }

ForStmt::ForStmt(Stmt* initializer, Expr* condition, Expr* increment, Stmt* body) : initializer(initializer), condition(condition), increment(increment), body(body) {}
void ForStmt::accept(Visitor* v) { v->visit(this); }

ReturnStmt::ReturnStmt(Expr* returnValue) : returnValue(returnValue) {}
void ReturnStmt::accept(Visitor* v) { v->visit(this); }

SendStmt::SendStmt(const std::string& variableName, const std::string& targetName) : variableName(variableName), targetName(targetName) {}
void SendStmt::accept(Visitor* v) { v->visit(this); }

RecvStmt::RecvStmt(const std::string& variableName, const std::string& srcFunction) : variableName(variableName), srcFunction(srcFunction) {}
void RecvStmt::accept(Visitor* v) { v->visit(this); }

BlockStmt::BlockStmt(const std::vector<Stmt*>& statements) : statements(statements) {}
void BlockStmt::accept(Visitor* v) { v->visit(this); }

ExprStmt::ExprStmt(Expr* expression) : expression(expression) {}
void ExprStmt::accept(Visitor* v) { v->visit(this); }

FunctionDecl::FunctionDecl(const std::string& name, const std::vector<Parameter>& parameters, Token returnType, Stmt* body) :
    name(name), parameters(parameters), returnType(returnType), body(body) {}
void FunctionDecl::accept(Visitor* v) { v->visit(this); }

Program::Program(const std::vector<FunctionDecl*>& functions) : functions(functions) {}
void Program::accept(Visitor* v) { v->visit(this); }