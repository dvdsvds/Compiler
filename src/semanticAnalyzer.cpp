#include "semanticAnalyzer.hpp"
#include <iostream>
#include <set>

SemanticAnalyzer::SemanticAnalyzer(SymbolTable* track_symbol) 
    : track_symbol(track_symbol), loop_depth(0), return_type{Token::INVALID, "", 0, 0} {};

void SemanticAnalyzer::add_error(const semanticError& error) {
    errors.push_back(error);
}

bool SemanticAnalyzer::has_errors() {
    return !errors.empty();
}

std::vector<SemanticAnalyzer::semanticError> SemanticAnalyzer::get_errors() {
    return errors; 
}

void SemanticAnalyzer::print_errors() {
    for(const auto& err : errors) {
        std::cout << "[ERROR] " << err.error_msg << " at line " << err.line << ", column " << err.column << std::endl;
    }
}

bool SemanticAnalyzer::check_type(const TokenData& expected, const TokenData& actual) {
    return expected.type == actual.type;
}

void SemanticAnalyzer::enter_function(const TokenData& ret_type, const std::string& func_name) {
    return_type = ret_type;
    curr_function_name = func_name;
    out_consumed.clear();
}

void SemanticAnalyzer::exit_function() {
    return_type.type = Token::INVALID;
    curr_function_name = "";
    out_consumed.clear();
}

bool SemanticAnalyzer::analyze(Program* root) {
    root->accept(this);
    return !has_errors();
}

void SemanticAnalyzer::visit(LiteralExpr* node) {
    switch(node->getTokenType()) {
        case Token::NUMBER:
            node->setType(Token::S32);
            break;

        case Token::TRUE:
        case Token::FALSE:
            node->setType(Token::BOOL);
            break;

        case Token::STRING:
            node->setType(Token::S8_ARRAY);
            break;

        case Token::CHAR:
            node->setType(Token::S8);
            break;

        case Token::NULL_KW:
            node->setType(Token::NULL_T);
            break;
        
        default:
            node->setType(Token::INVALID);
            break;
    }
};

void SemanticAnalyzer::visit(VariableExpr* node) { 
    std::string name = node->getName();
    Symbol* sym = track_symbol->lookup(name);
    if(sym == nullptr) {
        add_error({"Variable '" + name + "' is not declared", errorType::VARIABLE_UNDECLARED, node->get_line(), node->get_column()});
        return;
    }
    node->setType(sym->get_type().type);
    bool is_out = sym->is_out_variable();
    if(is_out) {
        if(out_consumed.find(name) != out_consumed.end() && out_consumed[name]) {
            add_error({"Variable '" + name + "' is already consumed", errorType::OUT_ALREADY_CONSUMED, node->get_line(), node->get_column()});
        } else {
            out_consumed[name] = true;
        }
    }
};

void SemanticAnalyzer::visit(BinaryExpr* node) { 
    node->getLeft()->accept(this);
    node->getRight()->accept(this);

    Token leftType = node->getLeft()->getType();
    Token rightType = node->getRight()->getType();

    if(leftType == Token::INVALID || rightType == Token::INVALID) {
        node->setType(Token::INVALID);
        return;
    }

    switch(node->getOP()) {
        case Token::PLUS:
        case Token::MINUS:
        case Token::STAR:
        case Token::SLASH:
        case Token::PERCENT:
            if(leftType == rightType) {
                if(leftType == Token::S8) {
                    node->setType(Token::S8);
                } else if(leftType == Token::S16) {
                    node->setType(Token::S16);
                } else if(leftType == Token::S32) {
                    node->setType(Token::S32);
                } else if(leftType == Token::US8) {
                    node->setType(Token::US8);
                } else if(leftType == Token::US16) {
                    node->setType(Token::US16);
                } else if(leftType == Token::US32) {
                    node->setType(Token::US32);
                } else {
                    add_error({"Type mismatch in arithmetic operation: incompatible type " + tokenToString(leftType), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                    node->setType(Token::INVALID);
                    return;
                }

            } else {
                add_error({"Type mismatch in arithmetic operation: operands must have same type, got " + tokenToString(leftType) + " and " + tokenToString(rightType), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                node->setType(Token::INVALID);
                return;
            }
            break;
        
        case Token::LT:
        case Token::GT:
        case Token::LE:
        case Token::GE:
            if(leftType == rightType) {
                if(leftType == Token::S8 || leftType == Token::S16 || leftType == Token::S32 || leftType == Token::US8 || leftType == Token::US16 || leftType == Token::US32) {
                    node->setType(Token::BOOL);
                } else {
                    add_error({"Type mismatch in comparison: expected integer types, got " + tokenToString(leftType), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                    node->setType(Token::INVALID);
                    return;
                }

            } else {
                add_error({"Type mismatch in comparison: operand types must match, got " + tokenToString(leftType) + " and " + tokenToString(rightType), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                node->setType(Token::INVALID);
                return;
            }
            break;

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
        case Token::SHR_EQ:
            if(leftType == rightType) {
                node->setType(leftType);
            } else {
                add_error({"Type mismatch in equality: operand types must match, got " + tokenToString(leftType) + " and " + tokenToString(rightType), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                node->setType(Token::INVALID);
                return;
            }
            break;

        case Token::EQ:
        case Token::NE:
            if(leftType == rightType) {
                node->setType(Token::BOOL);
            } else {
                add_error({"Type mismatch in equality: operand types must match, got " + tokenToString(leftType) + " and " + tokenToString(rightType),  errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                node->setType(Token::INVALID);
                return;
            }
            break;

        case Token::AND:
        case Token::OR:
            if((leftType == Token::BOOL) && (rightType == Token::BOOL)) {
                node->setType(Token::BOOL);
            } else {
                add_error({"Type mismatch in logical operation: both operands must be bool, got " + tokenToString(leftType) + " and " + tokenToString(rightType), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                node->setType(Token::INVALID);
                return;
            }
            break;
        
        case Token::BIT_AND:
        case Token::BIT_OR:
        case Token::BIT_XOR:
        case Token::SHL:
        case Token::SHR:
            if(leftType == rightType) {
                if(leftType == Token::S8) {
                    node->setType(Token::S8);
                } else if(leftType == Token::S16) {
                    node->setType(Token::S16);
                } else if(leftType == Token::S32) {
                    node->setType(Token::S32);
                } else if(leftType == Token::US8) {
                    node->setType(Token::US8);
                } else if(leftType == Token::US16) {
                    node->setType(Token::US16);
                } else if(leftType == Token::US32) {
                    node->setType(Token::US32);
                } else {
                    add_error({"Type mismatch in bitwise operation: incompatible type " + tokenToString(leftType), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                    node->setType(Token::INVALID);
                    return;
                }

            } else {
                add_error({"Type mismatch in bitwise operation: operands must have same type, got " + tokenToString(leftType) + " and " + tokenToString(rightType), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                node->setType(Token::INVALID);
                return;
            }
            break;
        
        default:
            add_error({"Unknown binary operator", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
            node->setType(Token::INVALID);
            break;
    }
    
};

void SemanticAnalyzer::visit(UnaryExpr* node) { 
    node->getOperand()->accept(this);
    Token operand_type = node->getOperand()->getType();
    if(operand_type == Token::INVALID) {
        node->setType(Token::INVALID);
        return;
    }

    switch(node->getOP()) {
        case Token::NOT:
            if(operand_type == Token::BOOL) {
                node->setType(Token::BOOL);
            } else {
                add_error({"Type mismatch: NOT operator requires bool type, got " + tokenToString(operand_type), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                node->setType(Token::INVALID);
                return;
            }
            break;

        case Token::MINUS:
        case Token::BIT_NOT:
        case Token::INC:
        case Token::DEC:
                switch (operand_type) {
                    case Token::S8: node->setType(Token::S8); break;
                    case Token::S16: node->setType(Token::S16); break;
                    case Token::S32: node->setType(Token::S32); break;
                    case Token::US8: node->setType(Token::US8); break;
                    case Token::US16: node->setType(Token::US16); break;
                    case Token::US32: node->setType(Token::US32); break;
                    default:
                        add_error({"Type mismatch: unary operator requires integer type, got " + tokenToString(operand_type), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                        node->setType(Token::INVALID);
                        return;
                }
            break;
        
        case Token::BIT_AND:
            if(VariableExpr* varExpr = dynamic_cast<VariableExpr*>(node->getOperand())) {
                Symbol* symbol = track_symbol->lookup(varExpr->getName());
                if(symbol) {
                    symbol->setAddressTaken(true);
                }
                switch(operand_type) {
                    case Token::S8: node->setType(Token::PS8); break;
                    case Token::S16: node->setType(Token::PS16); break;
                    case Token::S32: node->setType(Token::PS32); break;
                    case Token::US8: node->setType(Token::PUS8); break;
                    case Token::US16: node->setType(Token::PUS16); break;
                    case Token::US32: node->setType(Token::PUS32); break;
                    default:
                        add_error({"Address operator requires integer type variable, got " + tokenToString(operand_type), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                        node->setType(Token::INVALID);
                        return;
                }
            } else {
                add_error({"Address operator can only be applied to variables", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                node->setType(Token::INVALID);
            }
            break;
        default:
            add_error({"Unknown unary operator", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
            node->setType(Token::INVALID);
            break;
    }
};

void SemanticAnalyzer::visit(CallExpr* node) { 
    for(auto& argu : node->getArguments()) {
        argu->accept(this);
    }

    Symbol* functionName = track_symbol->lookup(node->getFunctionName());
    if(functionName == nullptr || !(functionName->is_func())) {
        add_error({"Function '" + node->getFunctionName() + "' is not declared", errorType::FUNCTION_UNDECLARED, node->get_line(), node->get_column()});
        node->setType(Token::INVALID);
        return;
    }

    if(node->getArguments().size() != functionName->get_param_types().size()) {
        add_error({"Function '" + node->getFunctionName() + "' expects " + std::to_string(functionName->get_param_types().size()) + " arguments, got " + std::to_string(node->getArguments().size()), errorType::PARAMETER_COUNT_INCONSISTENCY, node->get_line(), node->get_column()});
        node->setType(Token::INVALID);
        return;
    }

    for(size_t i = 0; i < node->getArguments().size(); i++) {
        if(node->getArguments()[i]->getType() != functionName->get_param_types()[i].type) {
            add_error({"Function '" + node->getFunctionName() + "' parameter " + std::to_string(i+1) + " type mismatch: expected " + tokenToString(functionName->get_param_types()[i].type) + ", got " + tokenToString(node->getArguments()[i]->getType()), errorType::PARAMETER_TYPE_INCONSISTENCY, node->get_line(), node->get_column()});
            node->setType(Token::INVALID);
            return;
        }     
    }

    node->setType(functionName->get_return_type().type);
};

void SemanticAnalyzer::visit(ArrayAccessExpr* node) { 
    node->getArrayName()->accept(this);
    node->getIndex()->accept(this);
    
    Token arrayType = node->getArrayName()->getType();
    Token indexType = node->getIndex()->getType();

    if(arrayType == Token::INVALID || indexType == Token::INVALID) {
        node->setType(Token::INVALID);
        return;
    }

    if(indexType != Token::S8 && indexType != Token::S16 && indexType != Token::S32 && indexType != Token::US8 && indexType != Token::US16 && indexType != Token::US32) {
        add_error({"Array index must be integer type, got " + tokenToString(indexType), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
        node->setType(Token::INVALID);
        return;
    }

    switch(arrayType) {
        case Token::S8_ARRAY:
            node->setType(Token::S8);
            break;
        case Token::S16_ARRAY:
            node->setType(Token::S16);
            break;
        case Token::S32_ARRAY:
            node->setType(Token::S32);
            break;
        case Token::US8_ARRAY:
            node->setType(Token::US8);
            break;
        case Token::US16_ARRAY:
            node->setType(Token::US16);
            break;
        case Token::US32_ARRAY:
            node->setType(Token::US32);
            break;
        case Token::BOOL_ARRAY:
            node->setType(Token::BOOL);
            break;
        default:
            add_error({"Array access on non-array type " + tokenToString(arrayType), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
            node->setType(Token::INVALID);
            break;
    }
};

void SemanticAnalyzer::visit(InExpr* node) {
    // Symbol* sym = track_symbol->lookup(node->getVariableName());
    // if(sym == nullptr) {
    //     add_error({"Variable '" + node->getVariableName() + "' is not declared", errorType::VARIABLE_UNDECLARED, node->get_line(), node->get_column()});
    //     node->setType(Token::INVALID);
    //     return;
    // }
    
    // if(out_consumed.find(node->getVariableName()) == out_consumed.end()) {
    //     add_error({"in() can only be used with out variables", errorType::IN_NON_OUT_VARIABLE, node->get_line(), node->get_column()});
    //     node->setType(Token::INVALID);
    //     return;
    // }
    
    // out_consumed[node->getVariableName()] = true;
    // node->setType(sym->get_type().type);
    if(out_consumed.find(node->getVariableName()) == out_consumed.end()) {
        add_error({"in() can only be used with out variables", errorType::IN_NON_OUT_VARIABLE, node->get_line(), node->get_column()});
        node->setType(Token::INVALID);
        return;
    }

    node->setType(Token::S32);
}

void SemanticAnalyzer::visit(OutExpr* node) {
    Symbol* sym = track_symbol->lookup(node->getVariableName());
    if(sym == nullptr) {
        add_error({"Variable '" + node->getVariableName() + "' is not declared", errorType::VARIABLE_UNDECLARED, node->get_line(), node->get_column()});
        node->setType(Token::INVALID);
        return;
    }

    out_consumed[node->getVariableName()] = false;
    
    node->setType(Token::VOID);
}

void SemanticAnalyzer::visit(ReferenceExpr* node) {
    Expr* inner = node->getExpr();
    inner->accept(this);

    Token base_type = Token::S32;

    if(VariableExpr* varExpr = dynamic_cast<VariableExpr*>(inner)) {
        Symbol* sym = track_symbol->lookup(varExpr->getName());
        if(!sym) {
            add_error({"Variable '" + varExpr->getName() + "' is not declared", errorType::VARIABLE_UNDECLARED, node->get_line(), node->get_column()});
            return;
        }
        base_type = sym->get_type().type;
    } else if(ArrayAccessExpr* arrAccess = dynamic_cast<ArrayAccessExpr*>(inner)) {
        base_type = arrAccess->getType();
    } else {
        add_error({"Cannot take address of this expression", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
        return;
    }

    Token ptr_type = Token::PS32;
    switch(base_type) {
        case Token::S8: ptr_type = Token::PS8; break;
        case Token::S16: ptr_type = Token::PS16; break;
        case Token::S32: ptr_type = Token::PS32; break;
        case Token::US8: ptr_type = Token::PUS8; break;
        case Token::US16: ptr_type = Token::PUS16; break;
        case Token::US32: ptr_type = Token::PUS32; break;
        default: break;
    }
    node->setType(ptr_type);
}

void SemanticAnalyzer::visit(DereferenceExpr* node) {
    node->getExpr()->accept(this);
    Token ptr_type = node->getExpr()->getType();
    Token base_type = Token::S32;
    switch(ptr_type) {
        case Token::PS8:  base_type = Token::S8; break;
        case Token::PS16: base_type = Token::S16; break;
        case Token::PS32: base_type = Token::S32; break;
        case Token::PUS8:  base_type = Token::US8; break;
        case Token::PUS16: base_type = Token::US16; break;
        case Token::PUS32: base_type = Token::US32; break;
        default: break;
    }
    node->setType(base_type);
}

void SemanticAnalyzer::visit(ArrayExpr* node) {
    for(auto* elem : node->getElements()) {
        elem->accept(this);
    }
    if(!node->getElements().empty()) {
        Token elem_type = node->getElements()[0]->getType();
        switch(elem_type) {
            case Token::S8:  node->setType(Token::S8_ARRAY); break;
            case Token::S16: node->setType(Token::S16_ARRAY); break;
            case Token::S32: node->setType(Token::S32_ARRAY); break;
            case Token::US8:  node->setType(Token::US8_ARRAY); break;
            case Token::US16: node->setType(Token::US16_ARRAY); break;
            case Token::US32: node->setType(Token::US32_ARRAY); break;
            case Token::BOOL: node->setType(Token::BOOL_ARRAY); break;
            default: break;
        }
    }
}

void SemanticAnalyzer::visit(VarDeclStmt* node) { 
    if(track_symbol->lookup_current_scope(node->getName()) != nullptr) {
        add_error({"Variable '" + node->getName() + "' is already declared in this scope", errorType::VARIABLE_REDECLARED, node->get_line(), node->get_column()});
        return;
    }

    if(node->getInitializer() != nullptr) {
        node->getInitializer()->accept(this);
        if(node->getInitializer()->getType() == Token::INVALID && !dynamic_cast<ArrayExpr*>(node->getInitializer())) {
            return;
        }

        if(node->getType() != node->getInitializer()->getType() && !dynamic_cast<ArrayExpr*>(node->getInitializer())) {
            add_error({"Variable '" + node->getName() + "' initialization type mismatch: expected " + tokenToString(node->getType()) + ", got " + tokenToString(node->getInitializer()->getType()), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
            return;
        }
    }

    TokenData varType = {node->getType(), "", node->get_line(), node->get_column()};
    track_symbol->insert(node->getName(), {node->getName(), varType, track_symbol->curr_scope(), false, false});

    Symbol* sym = track_symbol->lookup(node->getName());
    if(sym != nullptr) {
        node->setAddressTaken(sym->is_address_taken());
    }
};

void SemanticAnalyzer::visit(AssignStmt* node) { 
    if(dynamic_cast<VariableExpr*>(node->getTarget()) == nullptr && dynamic_cast<ArrayAccessExpr*>(node->getTarget()) == nullptr) {
        add_error({"Assignment target must be a variable or array element", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
        return;
    }
    node->getTarget()->accept(this);
    node->getValue()->accept(this);

    Token targetType = node->getTarget()->getType();
    Token valueType = node->getValue()->getType();

    if(targetType == Token::INVALID || valueType == Token::INVALID) {
        return;
    }

    if(targetType != valueType) {
        add_error({"Assignment type mismatch: cannot assign " + tokenToString(valueType) + " to " + tokenToString(targetType), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
    }
};

void SemanticAnalyzer::visit(IfStmt* node) { 
    node->getCondition()->accept(this);
    if(node->getCondition()->getType() == Token::INVALID) {
        return;
    }
    if(node->getCondition()->getType() != Token::BOOL) {
        add_error({"If condition must be bool type, got " + tokenToString(node->getCondition()->getType()), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
        return;
    }

    node->getThenBranch()->accept(this);

    if(node->getElseBranch() != nullptr) {
        node->getElseBranch()->accept(this);
    }
};

void SemanticAnalyzer::visit(LoopStmt* node) { 
    track_symbol->enter_scope();

    if(node->getInitializer() != nullptr) {
        node->getInitializer()->accept(this);
    }
    
    node->getCondition()->accept(this);
    if(node->getCondition()->getType() != Token::BOOL) {
        add_error({"Loop condition must be boolean type, got " + tokenToString(node->getCondition()->getType()), errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
        return;
    }

    if(node->getIncrement() != nullptr) {
        node->getIncrement()->accept(this);
    }

    bool prev_in_loop = in_loop;
    in_loop = true;
    node->getBody()->accept(this);
    in_loop = prev_in_loop;

    track_symbol->exit_scope();
};

void SemanticAnalyzer::visit(ReturnStmt* node) { 
    if(node->getReturnValue() != nullptr) {
        node->getReturnValue()->accept(this);
        if(return_type.type == Token::VOID) {
            add_error({"Cannot return a value from void function", errorType::RETURN_TYPE_INCONSISTENCY, node->get_line(), node->get_column()});
            return;
        } else if(return_type.type != node->getReturnValue()->getType()) {
            add_error({"Return type mismatch: expected " + tokenToString(return_type.type) + ", got " + tokenToString(node->getReturnValue()->getType()), errorType::RETURN_TYPE_INCONSISTENCY, node->get_line(), node->get_column()});
            return;
        }
    } else {
        if(return_type.type != Token::VOID) {
            add_error({"Function must return a value of type " + tokenToString(return_type.type), errorType::RETURN_TYPE_INCONSISTENCY, node->get_line(), node->get_column()});
            return;
        }
    }
};

void SemanticAnalyzer::visit(BreakStmt* node) { 
    if(!in_loop) {
        add_error({"break statment must be inside a loop", errorType::OUTSIDE_LOOP, node->get_line(), node->get_column()});
    }
}

void SemanticAnalyzer::visit(ContinueStmt* node) { 
    if(!in_loop) {
        add_error({"continue statment must be inside a loop", errorType::OUTSIDE_LOOP, node->get_line(), node->get_column()});
    }
}

void SemanticAnalyzer::visit(SendStmt* node) { 
    Symbol* varSymbol = track_symbol->lookup(node->getVariableName());
    if(varSymbol == nullptr) {
        add_error({"Variable '" + node->getVariableName() + "' is not declared", errorType::VARIABLE_UNDECLARED, node->get_line(), node->get_column()});
        return;
    }

    all_sends.push_back({
        node->getVariableName(),
        node->getTargetName(),
        node->get_line(),
        node->get_column(),
        node
    });
};

void SemanticAnalyzer::visit(RecvStmt* node) { 
    TokenData varType = {Token::S32, "", node->get_line(), node->get_column()};
    track_symbol->insert(node->getVariableName(), {node->getVariableName(), varType, track_symbol->curr_scope(), false, false});
    node->setVarType(Token::S32);
    function_recvs[curr_function_name].push_back(RecvInfo{
        node->getVariableName(),
        "",
        Token::S32,
        node->get_line(),
        node->get_column()
    });
};

void SemanticAnalyzer::visit(BlockStmt* node) { 
    track_symbol->enter_scope();
    for(auto& stats : node->getStatements()) {
        stats->accept(this);
    }
    track_symbol->exit_scope();
};

void SemanticAnalyzer::visit(ExprStmt* node) { 
    node->getExpression()->accept(this);
};

void SemanticAnalyzer::visit(ImportDecl* node) { }

void SemanticAnalyzer::visit(PrintStmt* node) {
    node->getExpression()->accept(this);
}

void SemanticAnalyzer::visit(FunctionDecl* node) { 
    track_symbol->enter_scope();
    
    enter_function({node->getReturnType(), "", node->get_line(), node->get_column()}, node->getName());
    
    for(const auto& param : node->getParameters()) {
        if(track_symbol->lookup_current_scope(param.name) != nullptr) {
            add_error({"Parameter '" + param.name + "' is already declared", errorType::VARIABLE_REDECLARED, node->get_line(), node->get_column()});
            continue;
        }
        TokenData paramType = {param.type, "", node->get_line(), node->get_column()};
        track_symbol->insert(param.name, {param.name, paramType, track_symbol->curr_scope(), false, false});
    }
    
    node->getBody()->accept(this);
    exit_function();
    track_symbol->exit_scope();
}

void SemanticAnalyzer::visit(Program* node) { 
    std::set<std::string> valid_functions;
    std::vector<TokenData> paramType;
    for(const auto& func : node->getFunctions()) {
        if(track_symbol->lookup_current_scope(func->getName()) != nullptr) {
            add_error({"Function '" + func->getName() + "' is already declared", errorType::FUNCTION_REDECLARED, node->get_line(), node->get_column()});
            continue;
        }

        paramType.clear();
        for(const auto& params : func->getParameters()) {
            paramType.push_back(TokenData{params.type, "", node->get_line(), node->get_column()});
        }

        TokenData returnType = TokenData{func->getReturnType(), "", node->get_line(), node->get_column()};
        track_symbol->insert(func->getName(), {func->getName(), returnType, paramType, track_symbol->curr_scope(), false});
        valid_functions.insert(func->getName());
    }

    for(const auto& func : node->getFunctions()) {
        if(valid_functions.find(func->getName()) != valid_functions.end()) {
            func->accept(this);
        }
    }

    std::set<std::string> sent_vars;
    std::set<std::string> recv_vars;

    for(const auto& send_info : all_sends) {
        sent_vars.insert(send_info.var_name);
    }
    for(const auto& kv : function_recvs) {
        for(const auto& recv : kv.second) {
            recv_vars.insert(recv.var_name);
        }
    }
    for(const auto& send_info : all_sends) {
        if(recv_vars.find(send_info.var_name) == recv_vars.end()) {
            add_error({"send('" + send_info.var_name + "') has no matching recv", errorType::NO_RECV_TARGET, send_info.line, send_info.column}); 
        }
    }
    for(const auto& kv : function_recvs) {
        for(const auto& recv : kv.second) {
            if(sent_vars.find(recv.var_name) == sent_vars.end()) {
                add_error({"recv('" + recv.var_name + "') has no matching send", errorType::NO_RECV_TARGET, recv.line, recv.column}); 
            }
        }
    }
}