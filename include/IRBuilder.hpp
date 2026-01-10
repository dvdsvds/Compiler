#pragma once
#include "IR.hpp"

class IRBuilder {
    private:
        IRModule* module;
        IRFunction* curr_function;
        BasicBlock* curr_block;
        int next_vreg;
        int next_label;
        std::map<std::string, Operand*> local_vars;

    public:
        IRBuilder(IRModule* module);
        Operand* newVirtualReg(Token data_type);
        std::string newLabel();
};