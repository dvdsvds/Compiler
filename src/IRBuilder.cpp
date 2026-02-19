#include "IRBuilder.hpp"

IRBuilder::IRBuilder(IRModule* module, SymbolTable* track_symbol) 
    : module(module), curr_function(nullptr), curr_block(nullptr), next_vreg(0), next_label(0), local_vars({}), track_symbol(track_symbol), in_loop_body(false) {}

Operand* IRBuilder::newVirtualReg(Token data_type) {
    return Operand::createVirtualReg(next_vreg++, data_type);
}
std::string IRBuilder::newLabel() {
    return "L" + std::to_string(next_label++);
}
void IRBuilder::enterScope() {
    local_vars_stack.push_back(local_vars);
}
void IRBuilder::exitScope() {
    if(!local_vars_stack.empty()) {
        local_vars = local_vars_stack.back();
        local_vars_stack.pop_back();
    }
}

Operand* IRBuilder::visitLiteralExpr(LiteralExpr* node) {
    Token tokenType = node->getTokenType();
    if(tokenType == Token::TRUE) {
        return Operand::createConstant(1, Token::BOOL);
    } else if(tokenType == Token::FALSE) {
        return Operand::createConstant(0, Token::BOOL);
    } else if(tokenType == Token::STRING) {
        std::string label = "_str_" + std::to_string(string_counter++);
        module->addStringLiteral(label, node->getValue());
        return Operand::createGlobal(label, Token::STRING);
    }
    return Operand::createConstant(std::stoi(node->getValue()), node->getType());
}
Operand* IRBuilder::visitVariableExpr(VariableExpr* node) {
    if(global_var_names.count(node->getName())) {
        uint32_t addr = global_addr_map[node->getName()];
        Operand* addr_op = Operand::createConstant(addr, Token::S32);
        Operand* result = newVirtualReg(Token::S32);
        curr_block->addInstruction(IRInstruction::createLoad(result, addr_op));
        return result;
    }
    if(var_address_taken[node->getName()]) {
        Operand* addr = local_vars[node->getName()];
        Operand* result = newVirtualReg(Token::S32);
        curr_block->addInstruction(IRInstruction::createLoad(result, addr));
        return result;
    }
    return local_vars[node->getName()];
}
Operand* IRBuilder::visitBinaryExpr(BinaryExpr* node) {
    Operand* left = evaluateExpr(node->getLeft());
    Operand* right = evaluateExpr(node->getRight());
    Operand* dest = newVirtualReg(node->getType());

    IRInstruction* instr;
    switch(node->getOP()) {
        case Token::PLUS: instr = IRInstruction::createAdd(dest, left, right); break;
        case Token::MINUS: instr = IRInstruction::createSub(dest, left, right); break;
        case Token::STAR: instr = IRInstruction::createMul(dest, left, right); break;
        case Token::SLASH: instr = IRInstruction::createDiv(dest, left, right); break;
        case Token::PERCENT: instr = IRInstruction::createMod(dest, left, right); break;
        case Token::EQ: instr = IRInstruction::createEq(dest, left, right); break;
        case Token::NE: instr = IRInstruction::createNe(dest, left, right); break;
        case Token::LT: instr = IRInstruction::createLt(dest, left, right); break;
        case Token::GT: instr = IRInstruction::createGt(dest, left, right); break;
        case Token::LE: instr = IRInstruction::createLe(dest, left, right); break;
        case Token::GE: instr = IRInstruction::createGe(dest, left, right); break;
        case Token::BIT_AND: instr = IRInstruction::createAnd(dest, left, right); break;
        case Token::BIT_OR: instr = IRInstruction::createOr(dest, left, right); break;
        case Token::BIT_XOR: instr = IRInstruction::createXor(dest, left, right); break;
        case Token::SHL: instr = IRInstruction::createShl(dest, left, right); break;
        case Token::SHR: instr = IRInstruction::createShr(dest, left, right); break;
        case Token::ASSIGN:
        case Token::PLUS_EQ:
        case Token::MINUS_EQ:
        case Token::STAR_EQ:
        case Token::SLASH_EQ:
        case Token::PERCENT_EQ:
        case Token::AND_EQ:
        case Token::OR_EQ:
        case Token::XOR_EQ:
        case Token::SHL_EQ:
        case Token::SHR_EQ: {
            Operand* result_value;
            if(node->getOP() == Token::ASSIGN) {
                result_value = right;
            } else {
                Operand* temp = newVirtualReg(node->getType());
                IRInstruction* op_instr = nullptr;
                switch(node->getOP()) {
                    case Token::PLUS_EQ: op_instr = IRInstruction::createAdd(temp, left, right); break;
                    case Token::MINUS_EQ: op_instr = IRInstruction::createSub(temp, left, right); break;
                    case Token::STAR_EQ: op_instr = IRInstruction::createMul(temp, left, right); break;
                    case Token::SLASH_EQ: op_instr = IRInstruction::createDiv(temp, left, right); break;
                    case Token::PERCENT_EQ: op_instr = IRInstruction::createMod(temp, left, right); break;
                    case Token::AND_EQ: op_instr = IRInstruction::createAnd(temp, left, right); break;
                    case Token::OR_EQ: op_instr = IRInstruction::createOr(temp, left, right); break;
                    case Token::XOR_EQ: op_instr = IRInstruction::createXor(temp, left, right); break;
                    case Token::SHL_EQ: op_instr = IRInstruction::createShl(temp, left, right); break;
                    case Token::SHR_EQ: op_instr = IRInstruction::createShr(temp, left, right); break;
                    default: break;
                }
                if(op_instr) curr_block->addInstruction(op_instr);
                result_value = temp;
            }
            
            if(VariableExpr* var = dynamic_cast<VariableExpr*>(node->getLeft())) {
                if(global_var_names.count(var->getName())) {
                    uint32_t addr = global_addr_map[var->getName()];
                    Operand* addr_op = Operand::createConstant(addr, Token::S32);
                    curr_block->addInstruction(IRInstruction::createStore(addr_op, result_value));
                } else {
                    bool is_addr_taken = var_address_taken[var->getName()];
                    if(is_addr_taken) {
                        Operand* addr = local_vars[var->getName()];
                        curr_block->addInstruction(IRInstruction::createStore(addr, result_value));
                    } else {
                        local_vars[var->getName()] = result_value;
                    }
                }
            } else if(DereferenceExpr* deref = dynamic_cast<DereferenceExpr*>(node->getLeft())) {
                Operand* addr = evaluateExpr(deref->getExpr());
                curr_block->addInstruction(IRInstruction::createStore(addr, result_value));
            } else if(ArrayAccessExpr* arrAccess = dynamic_cast<ArrayAccessExpr*>(node->getLeft())) {
                Operand* array_addr = evaluateExpr(arrAccess->getArrayName());
                Operand* index = evaluateExpr(arrAccess->getIndex());
                Operand* element_size = Operand::createConstant(4, Token::S32);
                Operand* offset = newVirtualReg(Token::S32);
                curr_block->addInstruction(IRInstruction::createMul(offset, index, element_size));
                Operand* actual_addr = newVirtualReg(Token::S32);
                curr_block->addInstruction(IRInstruction::createAdd(actual_addr, array_addr, offset));
                curr_block->addInstruction(IRInstruction::createStore(actual_addr, result_value));
            } else if(MemberAccessExpr* memberAccess = dynamic_cast<MemberAccessExpr*>(node->getLeft())) {
                Operand* addr = visitMemberAccessExprR(memberAccess);
                curr_block->addInstruction(IRInstruction::createStore(addr, result_value));
            }
            return result_value;
        }

        default: instr = nullptr; break;
    }

    if(instr) { curr_block->addInstruction(instr); }
    return dest;
}
Operand* IRBuilder::visitUnaryExpr(UnaryExpr* node) {
    Operand* operand = evaluateExpr(node->getOperand());
    Operand* dest = newVirtualReg(node->getType());

    IRInstruction* instr;
    switch(node->getOP()) { 
        case Token::MINUS: instr = IRInstruction::createNeg(dest, operand); break;
        case Token::NOT: instr = IRInstruction::createNot(dest, operand); break;
        case Token::BIT_NOT: instr = IRInstruction::createNot(dest, operand); break;
        default: instr = nullptr; break;
    }

    if(instr) { curr_block->addInstruction(instr); }
    return dest;
}
Operand* IRBuilder::visitCallExpr(CallExpr* node) {
    std::string func_name = node->getFunctionName();

    std::vector<Operand*> call_args;
    for(const auto& args : node->getArguments()) {
        if(VariableExpr* varExpr = dynamic_cast<VariableExpr*>(args)) {
            if(struct_var_types.count(varExpr->getName())) {
                call_args.push_back(local_vars[varExpr->getName()]);
                continue;
            }
        }
        call_args.push_back(evaluateExpr(args));
    }

    Operand* dest = newVirtualReg(node->getType());
    IRInstruction* instr = IRInstruction::createCall(dest, func_name, call_args);
    curr_block->addInstruction(instr); 
    return dest;
}
Operand* IRBuilder::visitArrayAccessExpr(ArrayAccessExpr* node) {
    Operand* array_addr = evaluateExpr(node->getArrayName());
    Operand* index = evaluateExpr(node->getIndex());

    int size = 0;
    switch(node->getType()) {
        case Token::S8: size = 1; break;
        case Token::S16: size = 2; break;
        case Token::S32: size = 4; break;
        case Token::US8: size = 1; break;
        case Token::US16: size = 2; break;
        case Token::US32: size = 4; break;
        case Token::BOOL: size = 1; break;
    }
    Operand* element_size = Operand::createConstant(size, node->getType());

    Operand* offset = newVirtualReg(node->getType());
    IRInstruction* mul_instr = IRInstruction::createMul(offset, index, element_size);
    curr_block->addInstruction(mul_instr);

    Operand* actual_addr = newVirtualReg(node->getType());
    IRInstruction* addr_instr = IRInstruction::createAdd(actual_addr, array_addr, offset);
    curr_block->addInstruction(addr_instr);

    Operand* dest = newVirtualReg(node->getType());
    IRInstruction* instr = IRInstruction::createLoad(dest, actual_addr);
    curr_block->addInstruction(instr);

    return dest;
}
Operand* IRBuilder::visitInExpr(InExpr* node) {
    if(out_values.find(node->getVariableName()) == out_values.end()) {
        return nullptr;
    }
    Operand* dest = Operand::createVirtualReg(next_vreg++, Token::S32);
    local_vars[node->getVariableName()] = dest;
    IRInstruction* in_instr = IRInstruction::createIn(dest);
    curr_block->addInstruction(in_instr);
    return dest;
}
Operand* IRBuilder::visitOutExpr(OutExpr* node) {
    Operand* value = local_vars[node->getVariableName()];
    out_values[node->getVariableName()] = value;
    IRInstruction* out_instr = IRInstruction::createOut(value);
    curr_block->addInstruction(out_instr);
    return nullptr;
}
Operand* IRBuilder::visitReferenceExpr(ReferenceExpr* node) {
    Expr* inner = node->getExpr();

    if(VariableExpr* varExpr = dynamic_cast<VariableExpr*>(inner)) {
        std::string name = varExpr->getName();
        if(!var_address_taken[name]) {
            Operand* old_var = local_vars[name];
            if(old_var->isConstant()) {
                Operand* temp = newVirtualReg(Token::S32);
                curr_block->addInstruction(IRInstruction::createCopy(temp, old_var));
                old_var = temp;
            }
            Operand* addr = newVirtualReg(Token::PS32);
            Operand* size = Operand::createConstant(4, Token::S32);
            curr_block->addInstruction(IRInstruction::createAlloca(addr, size));
            curr_block->addInstruction(IRInstruction::createStore(addr, old_var));
            local_vars[name] = addr;
            var_address_taken[name] = true;
        }
        return local_vars[name];
    } else if(ArrayAccessExpr* arrAccess = dynamic_cast<ArrayAccessExpr*>(inner)) {
        Operand* array_addr = evaluateExpr(arrAccess->getArrayName());
        Operand* index = evaluateExpr(arrAccess->getIndex());

        int size = 4;
        switch(arrAccess->getType()) {
            case Token::S8: size = 1; break;
            case Token::S16: size = 2; break;
            case Token::S32: size = 4; break;
            case Token::US8: size = 1; break;
            case Token::US16: size = 2; break;
            case Token::US32: size = 4; break;
            case Token::BOOL: size = 1; break;
            default: break;
        }
        Operand* element_size = Operand::createConstant(size, arrAccess->getType());
        Operand* offset = newVirtualReg(Token::S32);
        curr_block->addInstruction(IRInstruction::createMul(offset, index, element_size));

        Operand* actual_addr = newVirtualReg(Token::PS32);
        curr_block->addInstruction(IRInstruction::createAdd(actual_addr, array_addr, offset));

        return actual_addr;  // LOAD 없이 주소만 반환
    }

    return nullptr;
}
Operand* IRBuilder::visitDereferenceExpr(DereferenceExpr* node) {
        Operand* addr = evaluateExpr(node->getExpr());
        Operand* result = newVirtualReg(Token::PS32);
        curr_block->addInstruction(IRInstruction::createLoad(result, addr));
        return result;
}

Operand* IRBuilder::visitArrayExpr(ArrayExpr* node) {
    uint32_t size = node->getArraySize();
    int elem_size = 4;
    uint32_t total_size = size * elem_size;

    Operand* addr = newVirtualReg(Token::S32);
    Operand* alloca_size = Operand::createConstant(total_size, Token::S32);
    curr_block->addInstruction(IRInstruction::createAlloca(addr, alloca_size));

    auto elemtens = node->getElements();
    for(uint32_t i = 0; i < elemtens.size(); i++) {
        Operand* val = evaluateExpr(elemtens[i]);
        if(val->isConstant()) {
            Operand* temp = newVirtualReg(Token::S32);
            curr_block->addInstruction(IRInstruction::createCopy(temp, val));
            val = temp;
        }
        Operand* offset = Operand::createConstant(i * elem_size, Token::S32);
        Operand* elem_addr = newVirtualReg(Token::S32);
        curr_block->addInstruction(IRInstruction::createAdd(elem_addr, addr, offset));
        curr_block->addInstruction(IRInstruction::createStore(elem_addr, val));
    }

    return addr;
}

void IRBuilder::visitVarDeclStmt(VarDeclStmt* node) {
    std::string fname = node->getName();
    Token func_type = node->getType();

    if(func_type == Token::STRUCT_T) {
        std::string sname = node->getStructName();
        StructDecl* decl = struct_defs[sname];
        uint32_t num_members = decl->getMembers().size();

        Operand* addr = newVirtualReg(Token::S32);
        Operand* size_op = Operand::createConstant(num_members * 4, Token::S32);
        curr_block->addInstruction(IRInstruction::createAlloca(addr, size_op));

        if(node->getInitializer() != nullptr) {
            if(StructLiteralExpr* lit = dynamic_cast<StructLiteralExpr*>(node->getInitializer())) {
                auto values = lit->getValues();
                for(uint32_t i = 0; i < values.size(); i++) {
                    Operand* val = evaluateExpr(values[i]);
                    if(val->isConstant()) {
                        Operand* temp = newVirtualReg(Token::S32);
                        curr_block->addInstruction(IRInstruction::createCopy(temp, val));
                        val = temp;
                    }
                    Operand* offset = Operand::createConstant(i * 4, Token::S32);
                    Operand* member_addr = newVirtualReg(Token::S32);
                    curr_block->addInstruction(IRInstruction::createAdd(member_addr, addr, offset));
                    curr_block->addInstruction(IRInstruction::createStore(member_addr, val));
                }
            }
        } else {
            for(uint32_t i = 0; i < num_members; i++) {
                Operand* temp = newVirtualReg(Token::S32);
                curr_block->addInstruction(IRInstruction::createCopy(temp, Operand::createConstant(0, Token::S32)));
                Operand* offset = Operand::createConstant(i * 4, Token::S32);
                Operand* member_addr = newVirtualReg(Token::S32);
                curr_block->addInstruction(IRInstruction::createAdd(member_addr, addr, offset));
                curr_block->addInstruction(IRInstruction::createStore(member_addr, temp));
            }
        }

        local_vars[fname] = addr;
        var_address_taken[fname] = true;
        struct_var_types[fname] = sname;
        return;
    }

    Operand* init_value;
    if(node->getInitializer() != nullptr) {
        init_value = evaluateExpr(node->getInitializer());
    } else {
        init_value = Operand::createConstant(0, func_type);
    }

    bool is_addr_taken = node->isAddressTaken();

    int size = 0;
    switch(func_type) {
        case Token::S8: size = 1; break;
        case Token::S16: size = 2; break;
        case Token::S32: size = 4; break;
        case Token::US8: size = 1; break;
        case Token::US16: size = 2; break;
        case Token::US32: size = 4; break;
        case Token::BOOL: size = 1; break;
    }
    Operand* element_size = Operand::createConstant(size, func_type);

    if(is_addr_taken) {
        Operand* addr = newVirtualReg(node->getType());
        IRInstruction* alloca_instr = IRInstruction::createAlloca(addr, element_size);
        curr_block->addInstruction(alloca_instr);

        IRInstruction* store_instr = IRInstruction::createStore(addr, init_value);
        curr_block->addInstruction(store_instr);

        local_vars[fname] = addr;
    } else if(dynamic_cast<ArrayExpr*>(node->getInitializer())) {
        Operand* addr_slot = newVirtualReg(Token::PS32);
        Operand* slot_size = Operand::createConstant(4, Token::S32);
        curr_block->addInstruction(IRInstruction::createAlloca(addr_slot, slot_size));
        curr_block->addInstruction(IRInstruction::createStore(addr_slot, init_value));
        local_vars[fname] = addr_slot;
        var_address_taken[fname] = true;
    } else {
        local_vars[fname] = init_value; 
    }

    if(!dynamic_cast<ArrayExpr*>(node->getInitializer())) {
        var_address_taken[fname] = is_addr_taken;
    }
}
void IRBuilder::visitAssignStmt(AssignStmt* node) {
    Operand* target = evaluateExpr(node->getTarget());
    Operand* value = evaluateExpr(node->getValue());

    if(VariableExpr* var = dynamic_cast<VariableExpr*>(node->getTarget())) {
        if(global_var_names.count(var->getName())) {
            uint32_t addr = global_addr_map[var->getName()];
            Operand* addr_op = Operand::createConstant(addr, Token::S32);
            curr_block->addInstruction(IRInstruction::createStore(addr_op, value));
            return;
        }

        bool is_addr_taken = var_address_taken[var->getName()];

        if(is_addr_taken) {
            Operand* addr = local_vars[var->getName()];
            IRInstruction* store_instr = IRInstruction::createStore(addr, value);
            curr_block->addInstruction(store_instr);
        } else {
            local_vars[var->getName()] = value;
        }
    } else if(ArrayAccessExpr* arrAccess = dynamic_cast<ArrayAccessExpr*>(node->getTarget())) {
        Operand* array_addr = evaluateExpr(arrAccess->getArrayName());
        Operand* index = evaluateExpr(arrAccess->getIndex());

        int size = 4;
        Operand* element_size = Operand::createConstant(size, Token::S32);

        Operand* offset = newVirtualReg(Token::S32);
        curr_block->addInstruction(IRInstruction::createMul(offset, index, element_size));

        Operand* actual_addr = newVirtualReg(Token::S32);
        curr_block->addInstruction(IRInstruction::createAdd(actual_addr, array_addr, offset));
        
        curr_block->addInstruction(IRInstruction::createStore(actual_addr, value));
    }
}
void IRBuilder::visitIfStmt(IfStmt* node) {
    Operand* condition = evaluateExpr(node->getCondition());

    std::string then_label_str = newLabel();
    Operand* then_label = Operand::createLabel(then_label_str);
    std::string end_label_str = newLabel();
    Operand* end_label = Operand::createLabel(end_label_str);

    std::string else_label_str;
    Operand* else_label = nullptr;
    if(node->getElseBranch() != nullptr) {
        else_label_str = newLabel();
        else_label = Operand::createLabel(else_label_str);
    } else {
        else_label_str = end_label_str;
        else_label = end_label;
    }

    IRInstruction* branch_instr = IRInstruction::createBranch(condition, then_label);
    curr_block->addInstruction(branch_instr);

    curr_block->addInstruction(IRInstruction::createJump(else_label));

    BasicBlock* then_block = new BasicBlock(then_label_str);
    curr_function->addBasicBlock(then_block);
    curr_block = then_block;
    evaluateStmt(node->getThenBranch());
    if(curr_block->getInstructions().empty() || curr_block->getInstructions().back()->getOpcode() != IROpcode::JUMP) {
        curr_block->addInstruction(IRInstruction::createJump(end_label));
    }

    if(node->getElseBranch() != nullptr) {
        BasicBlock* else_block = new BasicBlock(else_label_str);
        curr_function->addBasicBlock(else_block);
        curr_block = else_block;
        evaluateStmt(node->getElseBranch());
        if(curr_block->getInstructions().empty() || curr_block->getInstructions().back()->getOpcode() != IROpcode::JUMP) {
            curr_block->addInstruction(IRInstruction::createJump(end_label));
        }
    } 

    BasicBlock* end_block = new BasicBlock(end_label_str);
    curr_function->addBasicBlock(end_block);
    curr_block = end_block;
}

void IRBuilder::visitLoopStmt(LoopStmt* node) {
    bool prev_in_loop_body = in_loop_body;
    in_loop_body = false;

    std::string entry_label_str = curr_block->getLabel();  
    std::string loop_label_str = newLabel();
    std::string body_label_str = newLabel();
    std::string end_label_str = newLabel();
    
    Operand* loop_label = Operand::createLabel(loop_label_str);
    Operand* body_label = Operand::createLabel(body_label_str);
    Operand* end_label = Operand::createLabel(end_label_str);
    
    if(node->getInitializer() != nullptr) {
        evaluateStmt(node->getInitializer());
    }
    
    std::set<std::string> modified_vars = findModifiedVars(node->getBody());
    if(node->getIncrement()) {
        if(BinaryExpr* binExpr = dynamic_cast<BinaryExpr*>(node->getIncrement())) {
            if(VariableExpr* var = dynamic_cast<VariableExpr*>(binExpr->getLeft())) {
                modified_vars.insert(var->getName());
            }
        }
    }
    
    std::map<std::string, Operand*> entry_values;
    std::set<std::string> valid_vars;
    for(const auto& var_name : modified_vars) {
        if(local_vars.find(var_name) != local_vars.end() && local_vars[var_name] != nullptr) {
            entry_values[var_name] = local_vars[var_name];
            valid_vars.insert(var_name);
        }
    }
    
    BasicBlock* entry_block = curr_block; 
    IRInstruction* entry_jump = IRInstruction::createJump(loop_label);
    curr_block->addInstruction(entry_jump);
    
    BasicBlock* loop_block = new BasicBlock(loop_label_str);
    curr_function->addBasicBlock(loop_block);
    
    entry_block->addSuccessor(loop_block);
    loop_block->addPredecessor(entry_block);
    
    curr_block = loop_block;
    
    std::map<std::string, IRInstruction*> phi_instructions;
    for(const auto& var_name : valid_vars) {
        Operand* phi_dest = newVirtualReg(Token::S32);
        IRInstruction* phi_instr = IRInstruction::createPhi(phi_dest, {});
        phi_instr->addPhiOperand(entry_values[var_name], entry_label_str);
        curr_block->addInstruction(phi_instr);
        phi_instructions[var_name] = phi_instr;
        local_vars[var_name] = phi_dest;
    }
    
    BasicBlock* body_block = new BasicBlock(body_label_str);
    curr_function->addBasicBlock(body_block);
    
    BasicBlock* end_block = new BasicBlock(end_label_str);
    curr_function->addBasicBlock(end_block);
    
    if(node->getCondition() != nullptr) {
        Operand* cond = evaluateExpr(node->getCondition());
        IRInstruction* branch_instr = IRInstruction::createBranch(cond, body_label);
        curr_block->addInstruction(branch_instr);
        IRInstruction* end_jump_instr = IRInstruction::createJump(end_label);
        curr_block->addInstruction(end_jump_instr);
        
        loop_block->addSuccessor(body_block);
        body_block->addPredecessor(loop_block);
        loop_block->addSuccessor(end_block);
        end_block->addPredecessor(loop_block);
    } else {
        IRInstruction* body_jump_instr = IRInstruction::createJump(body_label);
        curr_block->addInstruction(body_jump_instr);
        
        loop_block->addSuccessor(body_block);
        body_block->addPredecessor(loop_block);
    }
    
    curr_block = body_block;
    
    std::string prev_loop_end = current_loop_end_label;
    std::string prev_loop_start = current_loop_start_label;
    std::string prev_loop_continue = current_loop_continue_label;
    current_loop_end_label = end_label_str;
    current_loop_start_label = loop_label_str;
    auto prev_continue_infos = std::move(loop_continue_infos);
    loop_continue_infos.clear();
    

    std::string inc_label_str;
    if(node->getIncrement() != nullptr) {
        inc_label_str = newLabel();
        current_loop_continue_label = inc_label_str;
    } else {
        current_loop_continue_label = loop_label_str;
    }

    in_loop_body = true;
    evaluateStmt(node->getBody());
    in_loop_body = prev_in_loop_body;
    
    if(node->getIncrement() != nullptr) {
        BasicBlock* inc_block = new BasicBlock(inc_label_str);
        curr_function->addBasicBlock(inc_block);
        curr_block->addSuccessor(inc_block);
        inc_block->addPredecessor(curr_block);
        curr_block->addInstruction(IRInstruction::createJump(Operand::createLabel(inc_label_str)));

        std::string body_end_label = curr_block->getLabel();
        std::map<std::string, Operand*> body_end_values = local_vars;

        curr_block = inc_block;

        if(!loop_continue_infos.empty()) {
            for (const auto& var_name : valid_vars) {
                Operand* phi_dest = newVirtualReg(Token::S32);
                IRInstruction* phi_instr = IRInstruction::createPhi(phi_dest, {});
                phi_instr->addPhiOperand(body_end_values[var_name], body_end_label);
                for(auto& ci : loop_continue_infos) {
                    phi_instr->addPhiOperand(ci.second[var_name], ci.first->getLabel());
                    ci.first->addSuccessor(inc_block);
                    inc_block->addPredecessor(ci.first);
                }
                curr_block->addInstruction(phi_instr);
                local_vars[var_name] = phi_dest;
            }
        }

        evaluateExpr(node->getIncrement());
    } else if(!loop_continue_infos.empty()) {
        for(const auto& var_name : valid_vars) {
            for(auto& ci : loop_continue_infos) {
                phi_instructions[var_name]->addPhiOperand(ci.second[var_name], ci.first->getLabel());
                ci.first->addSuccessor(loop_block);
                loop_block->addPredecessor(ci.first);
            }
        }
    }
    
    current_loop_end_label = prev_loop_end;
    current_loop_start_label = prev_loop_start;
    current_loop_continue_label = prev_loop_continue;
    loop_continue_infos = std::move(prev_continue_infos);

    std::string back_edge_label = curr_block->getLabel();

    for(const auto& var_name : valid_vars) {
        phi_instructions[var_name]->addPhiOperand(local_vars[var_name], back_edge_label);
    }

    IRInstruction* jump_instr = IRInstruction::createJump(loop_label);
    curr_block->addInstruction(jump_instr);
    
    curr_block->addSuccessor(loop_block);
    loop_block->addPredecessor(curr_block);
    
    curr_block = end_block;

    for(const auto& var_name : valid_vars) {
        local_vars[var_name] = phi_instructions[var_name]->getDest();
    }
}
void IRBuilder::visitReturnStmt(ReturnStmt* node) {
    Operand* return_value = nullptr;
    if(node->getReturnValue() != nullptr) {
        return_value = evaluateExpr(node->getReturnValue());
    }

    IRInstruction* return_instr = IRInstruction::createReturn(return_value);
    curr_block->addInstruction(return_instr);
}
void IRBuilder::visitBreakStmt(BreakStmt* node) {
    Operand* target = Operand::createLabel(current_loop_end_label);
    curr_block->addInstruction(IRInstruction::createJump(target));
}
void IRBuilder::visitContinueStmt(ContinueStmt* node) {
    loop_continue_infos.push_back({curr_block, local_vars});
    Operand* target = Operand::createLabel(current_loop_continue_label);
    curr_block->addInstruction(IRInstruction::createJump(target));
}
void IRBuilder::visitSendStmt(SendStmt* node) {
    curr_block->addInstruction(IRInstruction::createSend(local_vars[node->getVariableName()], node->getTargetName()));
}
void IRBuilder::visitRecvStmt(RecvStmt* node) {
    Operand* dest = newVirtualReg(node->getVarType());
    IRInstruction* recv_instr = IRInstruction::createRecv(dest);
    curr_block->addInstruction(recv_instr);
    local_vars[node->getVariableName()] = dest;
}
void IRBuilder::visitBlockStmt(BlockStmt* node) {
    if(!in_loop_body) {
        enterScope();
    }
    for(const auto& statement : node->getStatements()) {
        evaluateStmt(statement);
    }
    if(!in_loop_body) {
        exitScope();
    }
}
void IRBuilder::visitExprStmt(ExprStmt* node) {
    evaluateExpr(node->getExpression());
}

void IRBuilder::visitImportDecl(ImportDecl* node) { }

void IRBuilder::visitStructDecl(StructDecl* node) {
    struct_defs[node->getName()] = node;
}
void IRBuilder::visitMemberAccessExpr(MemberAccessExpr* node) { }
Operand* IRBuilder::visitMemberAccessExprR(MemberAccessExpr* node) { 
    Operand* base_addr = nullptr;
    std::string structName;

    if(VariableExpr* varExpr = dynamic_cast<VariableExpr*>(node->getObject())) {
        base_addr = local_vars[varExpr->getName()];
        structName = struct_var_types[varExpr->getName()];
    } else {
        base_addr = evaluateExpr(node->getObject());
    }

    StructDecl* decl = struct_defs[structName];
    if(!decl) return nullptr;
    
    int member_index = -1;
    for(int i = 0; i < (int)decl->getMembers().size(); i++) {
        if(decl->getMembers()[i].name == node->getMemberName()) {
            member_index = i;
            break;
        }
    }
    if(member_index < 0) return nullptr;

    Operand* offset = Operand::createConstant(member_index * 4, Token::S32);
    Operand* member_addr = newVirtualReg(Token::S32);
    curr_block->addInstruction(IRInstruction::createAdd(member_addr, base_addr, offset));
    return member_addr;
}
void IRBuilder::visitStructLiteralExpr(StructLiteralExpr* node) { }

void IRBuilder::visitPrintStmt(PrintStmt* node) {
    Operand* value = evaluateExpr(node->getExpression());
    curr_block->addInstruction(IRInstruction::createPrint(value));
}

void IRBuilder::visitFunctionDecl(FunctionDecl* node) {
    std::vector<Operand*> params;
    for(const auto& parameter : node->getParameters()) {
        Operand* p_type = newVirtualReg(parameter.type);
        params.push_back(p_type);
        local_vars[parameter.name] = p_type;
        if(parameter.type == Token::STRUCT_T) {
            var_address_taken[parameter.name] = true;
            struct_var_types[parameter.name] = parameter.structName;
        }
    }

    IRFunction* func = new IRFunction(node->getName(), params, node->getReturnType());
    curr_function = func;

    BasicBlock* entry_block = new BasicBlock(node->getName());
    curr_function->addBasicBlock(entry_block);
    curr_block = entry_block;

    if(node->getName() == "main") {
        for(const auto& global : pending_globals_inits) {
            Operand* val = evaluateExpr(global->getInitializer());
            uint32_t addr = global_addr_map[global->getName()];
            Operand* addr_op = Operand::createConstant(addr, Token::S32);
            curr_block->addInstruction(IRInstruction::createStore(addr_op, val));
        }
    }

    evaluateStmt(node->getBody());
    if(node->getReturnType() == Token::VOID) {
        curr_block->addInstruction(IRInstruction::createReturn(nullptr));
    }
    module->addFunction(func);

    local_vars.clear();
    var_address_taken.clear();
}
void IRBuilder::visitProgram(Program* node) {
    for(const auto& s : node->getStructs()) {
        visitStructDecl(s);
    }

    for(const auto& global : node->getGlobals()) {
        global_addr_map[global->getName()] = global_base;
        global_var_names.insert(global->getName());
        global_base += 4;
        if(global->getInitializer() != nullptr) {
            pending_globals_inits.push_back(global);
        }
    }

    for(const auto& function : node->getFunctions()) {
        visitFunctionDecl(function);
    }
}

Operand* IRBuilder::evaluateExpr(Expr* expr) {
    if(LiteralExpr* lit = dynamic_cast<LiteralExpr*>(expr)) {
        return visitLiteralExpr(lit);
    } else if(VariableExpr* var = dynamic_cast<VariableExpr*>(expr)) {
        return visitVariableExpr(var);
    } else if(BinaryExpr* bin = dynamic_cast<BinaryExpr*>(expr)) {
        return visitBinaryExpr(bin);
    } else if(UnaryExpr* una = dynamic_cast<UnaryExpr*>(expr)) {
        return visitUnaryExpr(una);
    } else if(CallExpr* call = dynamic_cast<CallExpr*>(expr)) {
        return visitCallExpr(call);
    } else if(ArrayAccessExpr* array = dynamic_cast<ArrayAccessExpr*>(expr)) {
        return visitArrayAccessExpr(array);
    } else if(InExpr* in = dynamic_cast<InExpr*>(expr)) {
        return visitInExpr(in);
    } else if(OutExpr* out = dynamic_cast<OutExpr*>(expr)) {
        return visitOutExpr(out);
    } else if(ReferenceExpr* ref = dynamic_cast<ReferenceExpr*>(expr)) {
        return visitReferenceExpr(ref);
    } else if(DereferenceExpr* deref = dynamic_cast<DereferenceExpr*>(expr)) {
        return visitDereferenceExpr(deref);
    } else if(ArrayExpr* arr = dynamic_cast<ArrayExpr*>(expr)) {
        return visitArrayExpr(arr);
    } else if(MemberAccessExpr* member = dynamic_cast<MemberAccessExpr*>(expr)) {
        Operand* addr = visitMemberAccessExprR(member);
        Operand* result = newVirtualReg(member->getType());
        curr_block->addInstruction(IRInstruction::createLoad(result, addr));
        return result;
    }
    else {
        return nullptr;
    }
}

void IRBuilder::evaluateStmt(Stmt* stmt) {
    if(VarDeclStmt* var = dynamic_cast<VarDeclStmt*>(stmt)) {
        visitVarDeclStmt(var);
    } else if(AssignStmt* assign = dynamic_cast<AssignStmt*>(stmt)) {
        visitAssignStmt(assign);
    } else if(IfStmt* ifs = dynamic_cast<IfStmt*>(stmt)) {
        visitIfStmt(ifs);
    } else if(LoopStmt* loop = dynamic_cast<LoopStmt*>(stmt)) {
        visitLoopStmt(loop);
    } else if(ReturnStmt* returns = dynamic_cast<ReturnStmt*>(stmt)) {
        visitReturnStmt(returns);
    } else if(BreakStmt* bs = dynamic_cast<BreakStmt*>(stmt)) {
        visitBreakStmt(bs);
    } else if(ContinueStmt* cs = dynamic_cast<ContinueStmt*>(stmt)) {
        visitContinueStmt(cs);
    } else if(SendStmt* send = dynamic_cast<SendStmt*>(stmt)) {
        visitSendStmt(send);
    } else if(RecvStmt* recv = dynamic_cast<RecvStmt*>(stmt)) {
        visitRecvStmt(recv);
    } else if(BlockStmt* block = dynamic_cast<BlockStmt*>(stmt)) {
        visitBlockStmt(block);
    } else if(ExprStmt* expr = dynamic_cast<ExprStmt*>(stmt)) {
        visitExprStmt(expr);
    } else if(PrintStmt* print = dynamic_cast<PrintStmt*>(stmt)) {
        visitPrintStmt(print);
    }
}

std::set<std::string> IRBuilder::findModifiedVars(Stmt* stmt) {
    std::set<std::string> modified;
    
    if(VarDeclStmt* varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
        return modified;
    } else if(BlockStmt* block = dynamic_cast<BlockStmt*>(stmt)) {
        for(const auto& s : block->getStatements()) {
            auto vars = findModifiedVars(s);
            modified.insert(vars.begin(), vars.end());
        }
    } else if(IfStmt* ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        auto thenVars = findModifiedVars(ifStmt->getThenBranch());
        modified.insert(thenVars.begin(), thenVars.end());
        if(ifStmt->getElseBranch()) {
            auto elseVars = findModifiedVars(ifStmt->getElseBranch());
            modified.insert(elseVars.begin(), elseVars.end());
        }
    } else if(LoopStmt* loop = dynamic_cast<LoopStmt*>(stmt)) {
        if(loop->getInitializer()) {
            auto initVars = findModifiedVars(loop->getInitializer());
            modified.insert(initVars.begin(), initVars.end());
        }
        auto bodyVars = findModifiedVars(loop->getBody());
        modified.insert(bodyVars.begin(), bodyVars.end());
    } else if(ExprStmt* exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
        if(BinaryExpr* binExpr = dynamic_cast<BinaryExpr*>(exprStmt->getExpression())) {
            Token op = binExpr->getOP();
            if(op == Token::ASSIGN || op == Token::PLUS_EQ || op == Token::MINUS_EQ || 
               op == Token::STAR_EQ || op == Token::SLASH_EQ || op == Token::PERCENT_EQ ||
               op == Token::AND_EQ || op == Token::OR_EQ || op == Token::XOR_EQ ||
               op == Token::SHL_EQ || op == Token::SHR_EQ) {
                if(VariableExpr* var = dynamic_cast<VariableExpr*>(binExpr->getLeft())) {
                    modified.insert(var->getName());
                }
            }
        }
    }
    
    return modified;
}