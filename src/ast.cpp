#include "ast.hpp"
ASTNode::ASTNode(int32_t line, int32_t column) : line(line), column(column) {}
int32_t ASTNode::get_line() const { return line; }
int32_t ASTNode::get_column() const { return column; }

Expr::Expr(int32_t line, int32_t column) : ASTNode(line, column), exprType(Token::INVALID) {}
Token Expr::getType() const { return exprType; }
void Expr::setType(Token type) { exprType = type; }

Stmt::Stmt(int32_t line, int32_t column) : ASTNode(line, column) {}

LiteralExpr::LiteralExpr(Token tokenType, const std::string& value, int32_t line, int32_t column) 
    : Expr(line, column), tokenType(tokenType), value(value) {}
Token LiteralExpr::getTokenType() const { return tokenType; }
std::string LiteralExpr::getValue() const { return value; }
void LiteralExpr::accept(Visitor* v) { v->visit(this); }

VariableExpr::VariableExpr(const std::string& name, int32_t line, int32_t column) 
    : Expr(line, column), name(name) {}
std::string VariableExpr::getName() const { return name; }
void VariableExpr::accept(Visitor* v) { v->visit(this); }

BinaryExpr::BinaryExpr(Expr* left, Token op, Expr* right, int32_t line, int32_t column) 
    : Expr(line, column), left(left), op(op), right(right) {}
Expr* BinaryExpr::getLeft() const { return left; }
Expr* BinaryExpr::getRight() const { return right; }
Token BinaryExpr::getOP() const { return op; }
void BinaryExpr::accept(Visitor* v) { v->visit(this); }

UnaryExpr::UnaryExpr(Token op, Expr* operand, int32_t line, int32_t column) 
    : Expr(line, column), op(op), operand(operand) {}
Expr* UnaryExpr::getOperand() const { return operand; }
Token UnaryExpr::getOP() const { return op; }
void UnaryExpr::accept(Visitor* v) { v->visit(this); }

CallExpr::CallExpr(const std::string& functionName, const std::vector<Expr*>& arguments, int32_t line, int32_t column) 
    : Expr(line, column), functionName(functionName), arguments(arguments) {}
std::string CallExpr::getFunctionName() const { return functionName; }
std::vector<Expr*> CallExpr::getArguments() const { return arguments; }
void CallExpr::accept(Visitor* v) { v->visit(this); }

ArrayAccessExpr::ArrayAccessExpr(Expr* arrayName, Expr* index, int32_t line, int32_t column) 
    : Expr(line, column), arrayName(arrayName), index(index) {}
Expr* ArrayAccessExpr::getArrayName() const { return arrayName; }
Expr* ArrayAccessExpr::getIndex() const { return index; }
void ArrayAccessExpr::accept(Visitor* v) { v->visit(this); }

InExpr::InExpr(const std::string& variableName, int32_t line, int32_t column)
    : Expr(line, column), variableName(variableName) {}
std::string InExpr::getVariableName() const { return variableName; }
void InExpr::accept(Visitor* v) { v->visit(this); }

OutExpr::OutExpr(const std::string& variableName, int32_t line, int32_t column)
    : Expr(line, column), variableName(variableName) {}
std::string OutExpr::getVariableName() const { return variableName; }
void OutExpr::accept(Visitor* v) { v->visit(this); }

ReferenceExpr::ReferenceExpr(Expr* expr, int32_t line, int32_t column) 
    : Expr(line, column), expr(expr) {}
Expr* ReferenceExpr::getExpr() const { return expr; }
void ReferenceExpr::accept(Visitor* v) { v->visit(this); }

DereferenceExpr::DereferenceExpr(Expr* expr, int32_t line, int32_t column) 
    : Expr(line, column), expr(expr) {}
Expr* DereferenceExpr::getExpr() const { return expr; }
void DereferenceExpr::accept(Visitor* v) { v->visit(this); }

ArrayExpr::ArrayExpr(const std::vector<Expr*>& elements, uint32_t array_size, int32_t line, int32_t column)
    : Expr(line, column), elements(elements), array_size(array_size) {}
std::vector<Expr*> ArrayExpr::getElements() const { return elements; }
uint32_t ArrayExpr::getArraySize() const { return array_size; }
void ArrayExpr::accept(Visitor* v) { v->visit(this); }

VarDeclStmt::VarDeclStmt(Token type, const std::string& name, Expr* initializer, int32_t line, int32_t column) 
    : Stmt(line, column), type(type), name(name), initializer(initializer), address_taken(false) {}
Token VarDeclStmt::getType() const { return type; }
std::string VarDeclStmt::getName() const { return name; }
Expr* VarDeclStmt::getInitializer() const { return initializer; }
bool VarDeclStmt::isAddressTaken() const { return address_taken; }
void VarDeclStmt::setAddressTaken(bool taken) { address_taken = taken; }
void VarDeclStmt::accept(Visitor* v) { v->visit(this); }

AssignStmt::AssignStmt(Expr* target, Expr* value, int32_t line, int32_t column) 
    : Stmt(line, column), target(target), value(value) {}
Expr* AssignStmt::getTarget() const { return target; }
Expr* AssignStmt::getValue() const { return value; }
void AssignStmt::accept(Visitor* v) { v->visit(this); }

IfStmt::IfStmt(Expr* condition, Stmt* thenBranch, Stmt* elseBranch, int32_t line, int32_t column) 
    : Stmt(line, column), condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}
Expr* IfStmt::getCondition() const { return condition; }
Stmt* IfStmt::getThenBranch() const { return thenBranch; }
Stmt* IfStmt::getElseBranch() const { return elseBranch; }
void IfStmt::accept(Visitor* v) { v->visit(this); }

LoopStmt::LoopStmt(Stmt* initializer, Expr* condition, Expr* increment, Stmt* body, int32_t line, int32_t column)
    : Stmt(line, column), initializer(initializer), condition(condition), increment(increment), body(body) {}
Stmt* LoopStmt::getInitializer() const { return initializer;}
Expr* LoopStmt::getCondition() const { return condition; }
Expr* LoopStmt::getIncrement() const { return increment; }
Stmt* LoopStmt::getBody() const { return body; }
void LoopStmt::accept(Visitor* v) { v->visit(this); }

ReturnStmt::ReturnStmt(Expr* returnValue, int32_t line, int32_t column) 
    : Stmt(line, column), returnValue(returnValue) {}
Expr* ReturnStmt::getReturnValue() const { return returnValue; }
void ReturnStmt::accept(Visitor* v) { v->visit(this); }

BreakStmt::BreakStmt(int line, int col) : Stmt(line, col) {}
void BreakStmt::accept(Visitor* v) { v->visit(this); }

ContinueStmt::ContinueStmt(int line, int col) : Stmt(line, col) {}
void ContinueStmt::accept(Visitor* v) { v->visit(this); }

SendStmt::SendStmt(const std::string& variableName, const std::string& targetName, int32_t line, int32_t column) 
    : Stmt(line, column), variableName(variableName), targetName(targetName) {}
std::string SendStmt::getVariableName() const { return variableName; }
std::string SendStmt::getTargetName() const { return targetName; }
void SendStmt::accept(Visitor* v) { v->visit(this); }

RecvStmt::RecvStmt(const std::string& variableName, const std::string& srcFunction, int32_t line, int32_t column) 
    : Stmt(line, column), variableName(variableName), srcFunction(srcFunction), varType(Token::INVALID) {}
std::string RecvStmt::getVariableName() const { return variableName; }
std::string RecvStmt::getSrcFunction() const { return srcFunction; }
Token RecvStmt::getVarType() const { return varType; }
void RecvStmt::setVarType(Token type) { varType = type; }
void RecvStmt::accept(Visitor* v) { v->visit(this); }

BlockStmt::BlockStmt(const std::vector<Stmt*>& statements, int32_t line, int32_t column) 
    : Stmt(line, column), statements(statements) {}
std::vector<Stmt*> BlockStmt::getStatements() const { return statements; }
void BlockStmt::accept(Visitor* v) { v->visit(this); }

ExprStmt::ExprStmt(Expr* expression, int32_t line, int32_t column) 
    : Stmt(line, column), expression(expression) {}
Expr* ExprStmt::getExpression() const { return expression; }
void ExprStmt::accept(Visitor* v) { v->visit(this); }

PrintStmt::PrintStmt(Expr* expression, int32_t line, int32_t column) 
    : Stmt(line, column), expression(expression) {}
Expr* PrintStmt::getExpression() const { return expression; }
void PrintStmt::accept(Visitor* v) { v->visit(this); }

FunctionDecl::FunctionDecl(const std::string& name, const std::vector<Parameter>& parameters, Token returnType, Stmt* body, int32_t line, int32_t column) 
    : ASTNode(line, column), name(name), parameters(parameters), returnType(returnType), body(body) {}
std::string FunctionDecl::getName() const { return name; }
std::vector<Parameter> FunctionDecl::getParameters() const { return parameters; }
Token FunctionDecl::getReturnType() const { return returnType; }
Stmt* FunctionDecl::getBody() const { return body; }
void FunctionDecl::accept(Visitor* v) { v->visit(this); }

ImportDecl::ImportDecl(const std::string& moduleName, const std::vector<std::string>& symbols, int32_t line, int32_t column) 
    : ASTNode(line, column), moduleName(moduleName), symbols(symbols) {}
std::string ImportDecl::getModuleName() const { return moduleName; }
std::vector<std::string> ImportDecl::getSymbols() const { return symbols; }
bool ImportDecl::isSelectiveImport() const { return !symbols.empty(); }
void ImportDecl::accept(Visitor* v) { v->visit(this); }

Program::Program(const std::vector<ImportDecl*>& imports, const std::vector<VarDeclStmt*>& globals, const std::vector<FunctionDecl*>& functions, int32_t line, int32_t column) 
    : ASTNode(line, column), imports(imports), globals(globals), functions(functions) {}
std::vector<ImportDecl*> Program::getImports() const { return imports; }
std::vector<VarDeclStmt*> Program::getGlobals() const { return globals; }
std::vector<FunctionDecl*> Program::getFunctions() const { return functions; }
void Program::accept(Visitor* v) { v->visit(this); }
