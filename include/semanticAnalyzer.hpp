#pragma once
#include "symbolTable.hpp"

class SemantciAnalzyer {
    public:
        enum class errorType {
            VARIABLE_UNDECLARED,
            VARIABLE_REDECLARED,
            TYPE_MISMATCH,
            FUNCTION_UNDECLARED,
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
            int32_t line;
            int32_t column;
        };

    private:
        SymbolTable* track_symbol;
        std::vector<semanticError> errors;
        TokenData return_type;
        int32_t loop_depth;
        std::map<std::string, bool> out_consumed;

    public:
        SemantciAnalzyer(SymbolTable* track_symbol);
};