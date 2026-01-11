#pragma once
#include "IR.hpp"
#include "ast.hpp"

class IRBuilder {
    private:
        IRModule* module;
        IRFunction* curr_function;
        BasicBlock* curr_block;
        int next_vreg;
        int next_label;
        std::map<std::string, Operand*> local_vars;

        Operand* evaluateExpr(Expr* expr);

    public:
        IRBuilder(IRModule* module);
        Operand* newVirtualReg(Token data_type);
        std::string newLabel();

        Operand* visitLiteralExpr(LiteralExpr* node);
        Operand* visitVariableExpr(VariableExpr* node);
        Operand* visitBinaryExpr(BinaryExpr* node);
        Operand* visitUnaryExpr(UnaryExpr* node);
        Operand* visitCallExpr(CallExpr* node);
        Operand* visitArrayAccessExpr(ArrayAccessExpr* node);

        void visitVarDeclStmt(VarDeclStmt* node);
        void visitAssignStmt(AssignStmt* node);
        void visitIfStmt(IfStmt* node);
        void visitWhileStmt(WhileStmt* node);
        void visitForStmt(ForStmt* node);
        void visitReturnStmt(ReturnStmt* node);
        void visitSendStmt(SendStmt* node);
        void visitRecvStmt(RecvStmt* node);
        void visitBlockStmt(BlockStmt* node);
        void visitExprStmt(ExprStmt* node);

        void visitFunctionDecl(FunctionDecl* node);
        void visitProgram(Program* node);
};