#pragma once
#include <cstdint>
#include <vector>
#include "token.hpp"

class Symbol {
    public:
        Symbol(const std::string& name, const TokenData& type, int32_t scope_level, bool is_out);
        Symbol(const std::string& name, const TokenData& return_type, const std::vector<TokenData>& params, int32_t scope_level);

        enum class SymbolKind { VARIABLE, FUNCTION, PARAMETER };

        std::string get_name() const;
        TokenData get_type() const;
        TokenData get_return_type() const;
        std::vector<TokenData> get_param_types() const;
        SymbolKind get_kind() const;
        int32_t get_scope_level() const;
        bool is_out_variable() const;
        bool is_consumed() const;
        bool is_func() const;
        bool is_var() const;
        bool is_param() const;

        void mark_as_consumed();
    private:
        std::string name;
        TokenData type;
        SymbolKind kind;
        std::vector<TokenData> param_types;
        TokenData return_type;
        int32_t scope_level;
        bool is_out;
        bool consumed;
};