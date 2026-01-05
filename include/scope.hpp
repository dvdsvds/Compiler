#pragma once
#include <map>
#include "symbol.hpp"

class Scope {
    private:
        std::map<std::string, Symbol> symbol_repository;
        Scope* parent;
        int32_t scope_level;
    public:
        Scope(Scope* parent, int32_t scope_level);

        void insert(const std::string& name, const Symbol& symbol);
        Symbol* lookup(const std::string& name);
        bool exists(const std::string& name);

        Scope* get_parent() const;
        int32_t get_level() const;
};