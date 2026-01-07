#pragma once
#include "scope.hpp"

class SymbolTable {
    private:
        std::vector<Scope*> stack;
        int32_t curr_scope_level;
    public:
        SymbolTable();
        ~SymbolTable();

        void enter_scope();
        void exit_scope();
        void insert(const std::string& name, const Symbol& symbol);

        Symbol* lookup(const std::string& name);
        Symbol* lookup_current_scope(const std::string& name) const;
        int32_t curr_scope() const;
};