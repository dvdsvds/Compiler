#pragma once
#include <set>
#include "IR.hpp"
#include "ast.hpp"
#include "symbolTable.hpp"

class IRBuilder {
    private:
        SymbolTable* track_symbol;
        IRModule* module;
        IRFunction* curr_function;
        BasicBlock* curr_block;
        int next_vreg;
        int next_label;
        std::map<std::string, Operand*> local_vars;
        std::map<std::string, bool> var_address_taken;
        std::vector<std::map<std::string, Operand*>> local_vars_stack;
        std::map<std::string, Operand*> out_values;
        bool in_loop_body;

        std::set<std::string> findModifiedVars(Stmt* stmt);
        Operand* evaluateExpr(Expr* expr);
        void evaluateStmt(Stmt* stmt);

    public:
        IRBuilder(IRModule* module, SymbolTable* track_symbol);
        Operand* newVirtualReg(Token data_type);
        std::string newLabel();
        void enterScope();
        void exitScope();

        Operand* visitLiteralExpr(LiteralExpr* node);
        Operand* visitVariableExpr(VariableExpr* node);
        Operand* visitBinaryExpr(BinaryExpr* node);
        Operand* visitUnaryExpr(UnaryExpr* node);
        Operand* visitCallExpr(CallExpr* node);
        Operand* visitArrayAccessExpr(ArrayAccessExpr* node);
        Operand* visitInExpr(InExpr* node);
        Operand* visitOutExpr(OutExpr* node);

        void visitVarDeclStmt(VarDeclStmt* node);
        void visitAssignStmt(AssignStmt* node);
        void visitIfStmt(IfStmt* node);
        void visitLoopStmt(LoopStmt* node);
        void visitReturnStmt(ReturnStmt* node);
        void visitSendStmt(SendStmt* node);
        void visitRecvStmt(RecvStmt* node);
        void visitBlockStmt(BlockStmt* node);
        void visitExprStmt(ExprStmt* node);
        void visitPrintStmt(PrintStmt* node);

        void visitImportDecl(ImportDecl* node);
        void visitFunctionDecl(FunctionDecl* node);
        void visitProgram(Program* node);
};