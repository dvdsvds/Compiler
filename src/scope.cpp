#include "scope.hpp"

Scope::Scope(Scope* parent, int32_t scope_level) : parent(parent), scope_level(scope_level) {}

void Scope::insert(const std::string& name, const Symbol& symbol) {
    symbol_repository.insert(std::make_pair(name, symbol));
}

Symbol* Scope::lookup(const std::string& name) {
    auto value = symbol_repository.find(name);
    if(value != symbol_repository.end()) {
        return &(value->second);
    } 

    return nullptr;
}

bool Scope::exists(const std::string& name) {
    auto value = symbol_repository.find(name);
    if(value != symbol_repository.end()) {
        return true;
    }

    return false;
}

Scope* Scope::get_parent() const {
    return parent;
}

int32_t Scope::get_level() const {
    return scope_level;
}