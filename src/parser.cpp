#include "parser.hpp"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<TokenData>& tokens) : tokens(tokens), curr_pos(0) {}

Program* Parser::parse() {
    std::vector<FunctionDecl*> functions;
    while(!isAtEnd()) {
        functions.push_back(parseFunction());
    }
    return new Program(functions);
}
Token Parser::peek() {
    if(curr_pos >= tokens.size()) {
        return Token::EOF_TOKEN;
    }
    return tokens[curr_pos].type;
}
Token Parser::advance() {
    if(isAtEnd()) {
        return Token::EOF_TOKEN;
    }

    Token curr_type = tokens[curr_pos].type;
    curr_pos++;
    return curr_type;
}
bool Parser::check(Token type) {
    if(isAtEnd()) {
        return false;
    }

    if(peek() == type) {
        return true;
    }

    return false;
}
bool Parser::match(Token type) {
    if(check(type)) {
        advance();
        return true;
    }
    return false;
}
bool Parser::isAtEnd() {
    return curr_pos >= tokens.size() || peek() == Token::EOF_TOKEN;
}
void Parser::expect(Token type) {
    if(!check(type)) {
        std::string error = "Expected token type " + std::to_string(static_cast<int>(type)) + 
                           " but got " + std::to_string(static_cast<int>(peek())) + 
                           " at position " + std::to_string(curr_pos);
        throw std::runtime_error(error);
    }
    advance();
}
Expr* Parser::parseExpression() {
    return parseAssignment();
}
Expr* Parser::parseAssignment() {
    Expr* left = parseLogicalOr();
    Token op;
    if(match(Token::ASSIGN)){
        op = Token::ASSIGN;
    } else if(match(Token::PLUS_EQ)) {
        op = Token::PLUS_EQ;
    } else if(match(Token::MINUS_EQ)) {
        op = Token::MINUS_EQ;
    } else if(match(Token::STAR_EQ)) {
        op = Token::STAR_EQ;
    } else if(match(Token::SHL_EQ)) {
        op = Token::SHL_EQ;
    } else if(match(Token::SHR_EQ)) {
        op = Token::SHR_EQ;
    } else if(match(Token::AND_EQ)) {
        op = Token::AND_EQ;
    } else if(match(Token::OR_EQ)) {
        op = Token::OR_EQ;
    } else if(match(Token::XOR_EQ)) {
        op = Token::XOR_EQ;
    } else if(match(Token::SLASH_EQ)) {
        op = Token::SLASH_EQ;
    } else if(match(Token::PERCENT_EQ)) {
        op = Token::PERCENT_EQ;
    } else {
        return left;
    }

    Expr* right = parseAssignment();
    return new BinaryExpr(left, op, right);
}
Expr* Parser::parseLogicalOr() {
    Expr* left = parseLogicalAnd();
    while(match(Token::OR)) {
        Token op = Token::OR;
        Expr* right = parseLogicalAnd();
        left = new BinaryExpr(left, op, right);
    }

    return left;
} 
Expr* Parser::parseLogicalAnd() {
    Expr* left = parseBitwiseOr();
    while(match(Token::AND)) {
        Token op = Token::AND;
        Expr* right = parseBitwiseOr();
        left = new BinaryExpr(left, op, right);
    }

    return left;
}
Expr* Parser::parseBitwiseOr() {
    Expr* left = parseBitwiseXor();
    while(match(Token::BIT_OR)) {
        Token op = Token::BIT_OR;
        Expr* right = parseBitwiseXor();
        left = new BinaryExpr(left, op, right);
    }

    return left;
} 
Expr* Parser::parseBitwiseXor() {
    Expr* left = parseBitwiseAnd();
    while(match(Token::BIT_XOR)) {
        Token op = Token::BIT_XOR;
        Expr* right = parseBitwiseAnd();
        left = new BinaryExpr(left, op, right);
    }

    return left;
}
Expr* Parser::parseBitwiseAnd() {
    Expr* left = parseEquality();
    while(match(Token::BIT_AND)) {
        Token op = Token::BIT_AND;
        Expr* right = parseEquality();
        left = new BinaryExpr(left, op, right);
    }

    return left;
}
Expr* Parser::parseEquality() {
    Expr* left = parseRelational();
    while(true) {
        Token op;
        if(match(Token::EQ)) {
            op = Token::EQ;
        } else if(match(Token::NE)) {
            op = Token::NE;
        } else {
            break;
        }
        Expr* right = parseRelational();
        left = new BinaryExpr(left, op, right);
    }
    return left;
} 
Expr* Parser::parseRelational() {
    Expr* left = parseShift();
    while(true) {
        Token op;
        if(match(Token::LT)) {
            op = Token::LT;
        } else if(match(Token::GT)) {
            op = Token::GT;
        } else if(match(Token::LE)) {
            op = Token::LE;
        } else if(match(Token::GE)) {
            op = Token::GE;
        } else {
            break;
        }
        Expr* right = parseShift();
        left = new BinaryExpr(left, op, right);
    }

    return left;
}
Expr* Parser::parseShift() {
    Expr* left = parseAdditive();
    while(true) {
        Token op;
        if(match(Token::SHL)) {
            op = Token::SHL;
        } else if(match(Token::SHR)) {
            op = Token::SHR;
        } else {
            break;
        }

        Expr* right = parseAdditive();
        left = new BinaryExpr(left, op, right);
    }

    return left;
} 
Expr* Parser::parseAdditive() {
    Expr* left = parseMultiplicative();
    while(true) {
        Token op;
        if(match(Token::PLUS)) {
            op = Token::PLUS;
        } else if(match(Token::MINUS)) {
            op = Token::MINUS;
        } else {
            break;
        }

        Expr* right = parseMultiplicative();
        left = new BinaryExpr(left, op, right);
    }

    return left;
}
Expr* Parser::parseMultiplicative() {
    Expr* left = parseUnary();
    while(true) {
        Token op;
        if(match(Token::STAR)) {
            op = Token::STAR;
        } else if(match(Token::SLASH)) {
            op = Token::SLASH;
        } else if(match(Token::PERCENT)) {
            op = Token::PERCENT;
        } else {
            break;
        }

        Expr* right = parseUnary();
        left = new BinaryExpr(left, op, right);
    }

    return left;
} 
Expr* Parser::parseUnary() {
    Token op;
    if(match(Token::MINUS)) {
        op = Token::MINUS;
    } else if(match(Token::NOT)) {
        op = Token::NOT;
    } else if(match(Token::BIT_NOT)) {
        op = Token::BIT_NOT;
    } else if(match(Token::INC)) {
        op = Token::INC;
    } else if(match(Token::DEC)) {
        op = Token::DEC;
    } else {
        return parsePostfix();
    }
    Expr* operand = parseUnary();
    return new UnaryExpr(op, operand);
}   
Expr* Parser::parsePostfix() {
    Expr* left = parsePrimary();
    while(true) {
        if(match(Token::LPAREN)) {
            VariableExpr* varExpr = dynamic_cast<VariableExpr*>(left);
            if(!varExpr) {
                throw std::runtime_error("Can only call functions by name");
            }

            std::string funcName = varExpr->getName();

            std::vector<Expr*> args;
            if(!check(Token::RPAREN)) {
                while(true) {
                    args.push_back(parseExpression());
                    if(!match(Token::COMMA)) {
                        break;
                    }
                }
            }
            expect(Token::RPAREN);
            left = new CallExpr(funcName, args);
        } else if(match(Token::LBRACKET)) {
            Expr* index = parseExpression();
            expect(Token::RBRACKET);
            left = new ArrayAccessExpr(left, index);
        } else {
            break;
        }
    }

    return left;
}  
Expr* Parser::parsePrimary() {
    if(match(Token::NUMBER) || match(Token::STRING) || match(Token::CHAR) || 
       match(Token::TRUE) || match(Token::FALSE) || match(Token::NULL_KW)) {
        
        Token type = tokens[curr_pos - 1].type;
        std::string value = tokens[curr_pos - 1].value;

        return new LiteralExpr(type, value);
    } else if(match(Token::IDENTIFIER)) {
        std::string name = tokens[curr_pos - 1].value;

        return new VariableExpr(name);
    } else if(match(Token::LPAREN)) {
        Expr* expr = parseExpression();
        expect(Token::RPAREN);

        return expr;
    } else {
        throw std::runtime_error("Unexpected token in expression");
    }
} 
Stmt* Parser::parseStatement() {
    switch(peek()) {
        case Token::S8:
        case Token::S16:
        case Token::S32:
        case Token::US8:
        case Token::US16:
        case Token::US32:
        case Token::BOOL:
            return parseVardecl();

        case Token::IF:
            return parseIfStmt();
        
        case Token::WHILE:
            return parseWhileStmt();
        
        case Token::FOR:
            return parseForStmt();
        
        case Token::RETURN:
            return parseReturnStmt();
        
        case Token::SEND:
            return parseSendStmt();
        
        case Token::RECV:
            return parseRecvStmt();
        
        case Token::LBRACE:
            return parseBlock();
    
        default:
            return parseExprStmt();
    }
}
Stmt* Parser::parseVardecl() {
    Token type = advance(); 
    std::string variableName = tokens[curr_pos].value;
    expect(Token::IDENTIFIER);

    Expr* initializer = nullptr;
    if(match(Token::ASSIGN)) {
        initializer = parseExpression();
    }

    expect(Token::SEMICOLON);

    return new VarDeclStmt(type, variableName, initializer);
}
Stmt* Parser::parseIfStmt() {
    expect(Token::IF);
    expect(Token::LPAREN);
    Expr* condition = parseExpression();
    expect(Token::RPAREN);
    Stmt* blockThen = parseStatement();
    Stmt* blockElse = nullptr;
    if(match(Token::ELSE)) {
        blockElse = parseStatement();
    }

    return new IfStmt(condition, blockThen, blockElse);
}
Stmt* Parser::parseWhileStmt() {
    expect(Token::WHILE);
    expect(Token::LPAREN);
    Expr* condition = parseExpression();
    expect(Token::RPAREN);
    Stmt* body = parseStatement();

    return new WhileStmt(condition, body);
}
Stmt* Parser::parseForStmt() {
    expect(Token::FOR);
    expect(Token::LPAREN);

    Stmt* initializer = nullptr;
    Token typeToken = peek();
    switch(typeToken) {
        case Token::S8:
        case Token::S16:
        case Token::S32:
        case Token::US8:
        case Token::US16:
        case Token::US32:
        case Token::BOOL:
            initializer = parseVardecl();
            break;
    }

    Expr* condition = nullptr;
    if(!check(Token::SEMICOLON)) {
        condition = parseExpression();
    }
    expect(Token::SEMICOLON);

    Expr* increment = nullptr;
    if(!check(Token::RPAREN)) {
        increment = parseExpression();
    }
    expect(Token::RPAREN);

    Stmt* body = parseStatement();

    return new ForStmt(initializer, condition, increment, body);
}
Stmt* Parser::parseReturnStmt() {
    expect(Token::RETURN);

    Expr* returnValue = nullptr;
    if(!check(Token::SEMICOLON)) {
        returnValue = parseExpression();
    }
    expect(Token::SEMICOLON);

    return new ReturnStmt(returnValue);
}
Stmt* Parser::parseSendStmt() {
    expect(Token::SEND);
    expect(Token::LPAREN);

    std::string variableName = tokens[curr_pos].value;
    expect(Token::IDENTIFIER);
    expect(Token::RARROW);

    std::string functionName = tokens[curr_pos].value;
    expect(Token::IDENTIFIER);

    expect(Token::RPAREN);
    expect(Token::SEMICOLON);

    return new SendStmt(variableName, functionName);
}
Stmt* Parser::parseRecvStmt() {
    expect(Token::RECV);
    expect(Token::LPAREN);

    std::string srcFunction = tokens[curr_pos].value;
    expect(Token::IDENTIFIER);
    expect(Token::LARROW);

    std::string variableName = tokens[curr_pos].value;
    expect(Token::IDENTIFIER);

    expect(Token::RPAREN);
    expect(Token::SEMICOLON);

    return new RecvStmt(variableName, srcFunction);
}
Stmt* Parser::parseExprStmt() {
    Expr* expression = parseExpression();
    expect(Token::SEMICOLON);

    return new ExprStmt(expression);
}
Stmt* Parser::parseBlock() {
    expect(Token::LBRACE);

    std::vector<Stmt*> statements;
    while(!check(Token::RBRACE)) {
        statements.push_back(parseStatement());
    }
    
    expect(Token::RBRACE);

    return new BlockStmt(statements);
}
FunctionDecl* Parser::parseFunction() {
    if(isAtEnd()) {
        throw std::runtime_error("Expected function but got EOF");
    }

    std::string name = tokens[curr_pos].value;
    std::cout << "Function name: " << name << std::endl;
    expect(Token::IDENTIFIER);
    std::cout << "After IDENTIFIER" << std::endl;

    std::vector<Parameter> parameters;
    expect(Token::LPAREN);
    if(!check(Token::RPAREN)) {
        while(true) {
            Token parameterType = advance();
            std::string parameterName = tokens[curr_pos].value;
            expect(Token::IDENTIFIER);
            parameters.push_back(Parameter{parameterType, parameterName});

            if(!match(Token::COMMA)) {
                break;
            } else {
                continue;
            }
        }
    }

    expect(Token::RPAREN);
    expect(Token::RARROW);
    Token returnType = advance();
    Stmt* body = parseBlock();
    
    return new FunctionDecl(name, parameters, returnType, body);
}
