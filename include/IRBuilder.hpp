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
        int string_counter = 0;
        std::string current_loop_end_label;
        std::string current_loop_start_label;
        std::string current_loop_continue_label;
        std::vector<std::pair<BasicBlock*, std::map<std::string, Operand*>>> loop_continue_infos;
        std::map<std::string, uint32_t> global_addr_map;
        std::set<std::string> global_var_names;
        uint32_t global_base = 0x00100000;
        std::vector<VarDeclStmt*> pending_globals_inits;

        std::set<std::string> findModifiedVars(Stmt* stmt);
        Operand* evaluateExpr(Expr* expr);
        void evaluateStmt(Stmt* stmt);

        std::map<std::string, StructDecl*> struct_defs;
        std::map<std::string, std::string> struct_var_types;
        std::string curr_return_struct_name;

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
        Operand* visitReferenceExpr(ReferenceExpr* node);
        Operand* visitDereferenceExpr(DereferenceExpr* node);
        Operand* visitArrayExpr(ArrayExpr* node);

        void visitVarDeclStmt(VarDeclStmt* node);
        void visitAssignStmt(AssignStmt* node);
        void visitIfStmt(IfStmt* node);
        void visitLoopStmt(LoopStmt* node);
        void visitReturnStmt(ReturnStmt* node);
        void visitBreakStmt(BreakStmt* node);
        void visitContinueStmt(ContinueStmt* node);
        void visitSendStmt(SendStmt* node);
        void visitRecvStmt(RecvStmt* node);
        void visitBlockStmt(BlockStmt* node);
        void visitExprStmt(ExprStmt* node);
        void visitPrintStmt(PrintStmt* node);

        void visitImportDecl(ImportDecl* node);
        void visitStructDecl(StructDecl* node);
        void visitMemberAccessExpr(MemberAccessExpr* node);
        Operand* visitMemberAccessExprR(MemberAccessExpr* node);
        void visitStructLiteralExpr(StructLiteralExpr* node);
        void visitFunctionDecl(FunctionDecl* node);
        void visitProgram(Program* node);
};