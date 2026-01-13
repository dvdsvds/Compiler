#pragma once
#include "symbolTable.hpp"
#include "visitor.hpp"
#include "ast.hpp"

class SemanticAnalyzer : public Visitor {
    public:
        struct SendInfo {
            std::string var_name;
            std::string target_function;
            int32_t line, column;
            SendStmt* node;
        };

        struct RecvInfo {
            std::string var_name;
            std::string src_function;
            Token type;
            int32_t line, column;
        };

        enum class errorType {
            VARIABLE_UNDECLARED,
            VARIABLE_REDECLARED,
            TYPE_MISMATCH,
            FUNCTION_UNDECLARED,
            FUNCTION_REDECLARED,
            PARAMETER_COUNT_INCONSISTENCY,
            PARAMETER_TYPE_INCONSISTENCY,
            RETURN_TYPE_INCONSISTENCY,
            OUT_ALREADY_CONSUMED,
            IN_NON_OUT_VARIABLE,
            UNUSED_OUT,
            SEND_RECV_TYPE_INCONSISTENCY,
            NO_RECV_TARGET,
            OUTSIDE_LOOP,
            RETURN_OUTSIDE,
        };

        struct semanticError {
            std::string error_msg;
            errorType type;   
            int32_t line, column;
        };

    private:
        SymbolTable* track_symbol;
        std::vector<semanticError> errors;
        TokenData return_type;
        int32_t loop_depth;
        std::map<std::string, bool> out_consumed;
        std::map<std::string, std::vector<RecvInfo>> function_recvs;
        std::vector<SendInfo> all_sends;
        std::string curr_function_name;

    public:
        SemanticAnalyzer(SymbolTable* track_symbol);
        bool analyze(Program* root);

        void add_error(const semanticError& error);
        bool has_errors();
        std::vector<semanticError> get_errors();
        void print_errors();

        bool check_type(const TokenData& expected, const TokenData& actual);
        void enter_function(const TokenData& ret_type, const std::string& func_name); 
        void exit_function();

        void visit(LiteralExpr* node) override;
        void visit(VariableExpr* node) override;
        void visit(BinaryExpr* node) override;
        void visit(UnaryExpr* node) override;
        void visit(CallExpr* node) override;
        void visit(ArrayAccessExpr* node) override;
        void visit(VarDeclStmt* node) override;
        void visit(AssignStmt* node) override;
        void visit(IfStmt* node) override;
        void visit(LoopStmt* node) override;
        void visit(ReturnStmt* node) override;
        void visit(SendStmt* node) override;
        void visit(RecvStmt* node) override;
        void visit(BlockStmt* node) override;
        void visit(ExprStmt* node) override;
        void visit(FunctionDecl* node) override;
        void visit(Program* node) override;
};