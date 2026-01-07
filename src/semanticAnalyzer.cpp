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

void SemanticAnalyzer::enter_function(const TokenData& ret_type) {
    return_type = ret_type;
    out_consumed.clear();
}

void SemanticAnalyzer::exit_function() {
    return_type.type = Token::INVALID;
    out_consumed.clear();
}

bool SemanticAnalyzer::analyze(Program* root) {
    root->accept(this);
    return !has_errors();
}

void SemanticAnalyzer::visit(LiteralExpr* node) {};

void SemanticAnalyzer::visit(VariableExpr* node) { 
    std::string name = node->getName();
    Symbol* sym = track_symbol->lookup(name);
    if(sym == nullptr) {
        add_error({"", errorType::VARIABLE_UNDECLARED, node->get_line(), node->get_column()});
        return;
    }

    bool is_out = sym->is_out_variable();
    if(is_out) {
        if(out_consumed.find(name) != out_consumed.end() && out_consumed[name]) {
            add_error({"", errorType::OUT_ALREADY_CONSUMED, node->get_line(), node->get_column()});
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
                    add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                    node->setType(Token::INVALID);
                    return;
                }

            } else {
                add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
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
                    add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                    node->setType(Token::INVALID);
                    return;
                }

            } else {
                add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                node->setType(Token::INVALID);
                return;
            }
            break;
        
        case Token::EQ:
        case Token::NE:
            if(leftType == rightType) {
                node->setType(Token::BOOL);
            } else {
                add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                node->setType(Token::INVALID);
                return;
            }
            break;
        
        case Token::AND:
        case Token::OR:
            if((leftType == Token::BOOL) && (rightType == Token::BOOL)) {
                node->setType(Token::BOOL);
            } else {
                add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
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
                    add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                    node->setType(Token::INVALID);
                    return;
                }

            } else {
                add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                node->setType(Token::INVALID);
                return;
            }
            break;
        
        default:
            add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
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
                add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                node->setType(Token::INVALID);
                return;
            }
            break;

        case Token::MINUS:
        case Token::BIT_NOT:
        case Token::INC:
        case Token::DEC:
            if(operand_type == Token::S8) {
                node->setType(Token::S8);
            } else if(operand_type == Token::S16) {
                node->setType(Token::S16);
            } else if(operand_type == Token::S32) {
                node->setType(Token::S32);
            } else if(operand_type == Token::US8) {
                node->setType(Token::US8);
            } else if(operand_type == Token::US16) {
                node->setType(Token::US16);
            } else if(operand_type == Token::US32) {
                node->setType(Token::US32);
            } else {
                add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
                node->setType(Token::INVALID);
                return;
            }
            break;
        default:
            add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
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
        add_error({"", errorType::FUNCTION_UNDECLARED, node->get_line(), node->get_column()});
        node->setType(Token::INVALID);
        return;
    }

    if(node->getArguments().size() != functionName->get_param_types().size()) {
        add_error({"", errorType::PARAMETER_COUNT_INCONSISTENCY, node->get_line(), node->get_column()});
        node->setType(Token::INVALID);
        return;
    }

    for(size_t i = 0; i < node->getArguments().size(); i++) {
        if(node->getArguments()[i]->getType() != functionName->get_param_types()[i].type) {
            add_error({"", errorType::PARAMETER_TYPE_INCONSISTENCY, node->get_line(), node->get_column()});
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
        add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
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
            add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
            node->setType(Token::INVALID);
            break;
    }
};

void SemanticAnalyzer::visit(VarDeclStmt* node) { 
    if(track_symbol->lookup_current_scope(node->getName()) != nullptr) {
        add_error({"", errorType::VARIABLE_REDECLARED, node->get_line(), node->get_column()});
        return;
    }

    if(node->getInitializer() != nullptr) {
        node->getInitializer()->accept(this);
        if(node->getInitializer()->getType() == Token::INVALID) {
            return;
        }

        if(node->getType() != node->getInitializer()->getType()) {
            add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
            return;
        }
    }

    TokenData varType = {node->getType(), "", node->get_line(), node->get_column()};
    track_symbol->insert(node->getName(), {node->getName(), varType, track_symbol->curr_scope(), false});
};

void SemanticAnalyzer::visit(AssignStmt* node) { 
    if(dynamic_cast<VariableExpr*>(node->getTarget()) == nullptr && dynamic_cast<ArrayAccessExpr*>(node->getTarget()) == nullptr) {
        add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
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
        add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
    }
};

void SemanticAnalyzer::visit(IfStmt* node) { 
    node->getCondition()->accept(this);
    if(node->getCondition()->getType() == Token::INVALID) {
        return;
    }
    if(node->getCondition()->getType() != Token::BOOL) {
        add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
        return;
    }

    node->getThenBranch()->accept(this);

    if(node->getElseBranch() != nullptr) {
        node->getElseBranch()->accept(this);
    }
};

void SemanticAnalyzer::visit(WhileStmt* node) { 
    loop_depth++;
    node->getCondition()->accept(this);
    if(node->getCondition()->getType() == Token::INVALID) {
        loop_depth--;
        return;
    }
    
    if(node->getCondition()->getType() != Token::BOOL) {
        add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
        loop_depth--;
        return;
    }

    node->getBody()->accept(this);
    loop_depth--;
};

void SemanticAnalyzer::visit(ForStmt* node) { 
    track_symbol->enter_scope();
    if(node->getInitializer() != nullptr) {
        node->getInitializer()->accept(this);
    }

    loop_depth++;
    if(node->getCondition() != nullptr) {
        node->getCondition()->accept(this);
        if(node->getCondition()->getType() == Token::INVALID) {
            loop_depth--;
            track_symbol->exit_scope();
            return;
        }
        if(node->getCondition()->getType() != Token::BOOL) {
            add_error({"", errorType::TYPE_MISMATCH, node->get_line(), node->get_column()});
            loop_depth--;
            track_symbol->exit_scope();
            return;
        }
    }

    if(node->getIncrement() != nullptr) {
        node->getIncrement()->accept(this);
    }

    node->getBody()->accept(this);
    loop_depth--;
    track_symbol->exit_scope();
};

void SemanticAnalyzer::visit(ReturnStmt* node) { 
    if(node->getReturnValue() != nullptr) {
        node->getReturnValue()->accept(this);
        if(return_type.type == Token::VOID) {
            add_error({"", errorType::RETURN_TYPE_INCONSISTENCY, node->get_line(), node->get_column()});
            return;
        } else if(return_type.type != node->getReturnValue()->getType()) {
            add_error({"", errorType::RETURN_TYPE_INCONSISTENCY, node->get_line(), node->get_column()});
            return;
        }
    } else {
        if(return_type.type != Token::VOID) {
            add_error({"", errorType::RETURN_TYPE_INCONSISTENCY, node->get_line(), node->get_column()});
            return;
        }
    }
};

void SemanticAnalyzer::visit(SendStmt* node) { 
};
void SemanticAnalyzer::visit(RecvStmt* node) { 
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

void SemanticAnalyzer::visit(FunctionDecl* node) { 
    track_symbol->enter_scope();
    
    enter_function({node->getReturnType(), "", node->get_line(), node->get_column()});
    
    for(const auto& param : node->getParameters()) {
        if(track_symbol->lookup_current_scope(param.name) != nullptr) {
            add_error({"", errorType::VARIABLE_REDECLARED, node->get_line(), node->get_column()});
            continue;
        }
        TokenData paramType = {param.type, "", node->get_line(), node->get_column()};
        track_symbol->insert(param.name, {param.name, paramType, track_symbol->curr_scope(), false});
    }
    
    node->getBody()->accept(this);
    exit_function();
    track_symbol->exit_scope();
}

void SemanticAnalyzer::visit(Program* node) { 
    std::set<std::string> valid_functions;
    
    for(const auto& func : node->getFunctions()) {
        if(track_symbol->lookup_current_scope(func->getName()) != nullptr) {
            add_error({"", errorType::FUNCTION_REDECLARED, func->get_line(), func->get_column()});
            continue;
        }
        
        std::vector<TokenData> paramsType;
        for(const auto& param : func->getParameters()) {
            paramsType.push_back({param.type, "", func->get_line(), func->get_column()});
        }
        TokenData retType = {func->getReturnType(), "", func->get_line(), func->get_column()};
        track_symbol->insert(func->getName(), {func->getName(), retType, paramsType, track_symbol->curr_scope()});
        valid_functions.insert(func->getName());
    }
    
    for(const auto& func : node->getFunctions()) {
        if(valid_functions.count(func->getName()) > 0) {
            func->accept(this);
        }
    }
}