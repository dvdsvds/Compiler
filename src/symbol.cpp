#include "symbol.hpp"

Symbol::Symbol(const std::string& name, const TokenData& type, int32_t scope_level, bool is_out)
    : name(name), type(type), kind(SymbolKind::VARIABLE), scope_level(scope_level), is_out(is_out), consumed(false) {}

Symbol::Symbol(const std::string& name, const TokenData& return_type, const std::vector<TokenData>& params, int32_t scope_level)
    : name(name), return_type(return_type), param_types(params), kind(SymbolKind::FUNCTION), scope_level(scope_level), consumed(false) {}

std::string Symbol::get_name() const {
    return name;
}

TokenData Symbol::get_type() const {
    return type;
}

TokenData Symbol::get_return_type() const {
    return return_type;
}

std::vector<TokenData> Symbol::get_param_types() const {
    return param_types;
}

Symbol::SymbolKind Symbol::get_kind() const {
    return kind;
}

int32_t Symbol::get_scope_level() const {
    return scope_level;
}

bool Symbol::is_out_variable() const {
    return is_out;
}

bool Symbol::is_consumed() const {
    return consumed;
}

bool Symbol::is_func() const {
    return kind == SymbolKind::FUNCTION;
}

bool Symbol::is_var() const {
    return kind == SymbolKind::VARIABLE;
}

bool Symbol::is_param() const {
    return kind == SymbolKind::PARAMETER;
}

void Symbol::mark_as_consumed() {
    consumed = true;
}