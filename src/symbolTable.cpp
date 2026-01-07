#include "symbolTable.hpp"

SymbolTable::SymbolTable() {
    curr_scope_level = 0;
    stack.push_back(new Scope(nullptr, 0));
}

SymbolTable::~SymbolTable() {
    while(!stack.empty()) {
        delete stack.back();
        stack.pop_back();
    }
}

void SymbolTable::enter_scope() {
    stack.push_back(new Scope(stack.back(), curr_scope_level + 1));
    curr_scope_level++;
}

void SymbolTable::exit_scope() {
    delete stack.back();
    stack.pop_back();
    curr_scope_level--;
}

void SymbolTable::insert(const std::string& name, const Symbol& symbol) {
    stack.back()->insert(name, symbol);
}

Symbol* SymbolTable::lookup(const std::string& name) {
    Scope* curr_scope = stack.back();
    while(curr_scope != nullptr) {
        auto it = curr_scope->lookup(name);
        if(it != nullptr) {
            return it;
        } 
        curr_scope = curr_scope->get_parent();
    }

    return nullptr;
}

Symbol* SymbolTable::lookup_current_scope(const std::string& name) const {
    if(stack.empty()) {
        return nullptr;
    }
    return stack.back()->lookup(name);
}

int32_t SymbolTable::curr_scope() const {
    return curr_scope_level;
}