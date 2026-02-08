#include <stdexcept>
#include "parser.hpp"

Parser::Parser(const std::vector<TokenData>& tokens) : tokens(tokens), curr_pos(0) {}

Program* Parser::parse() {
    std::vector<ImportDecl*> imports;
    std::vector<FunctionDecl*> functions;

    while(check(Token::IMPORT)) {
        imports.push_back(parseImport());
    }

    while(!isAtEnd()) {
        functions.push_back(parseFunction());
    }

    return new Program(imports, functions, 0, 0);
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
        throw std::runtime_error("Expected token type " + std::to_string(static_cast<int>(type)) + " but got " + std::to_string(static_cast<int>(peek())) + " at position " + std::to_string(curr_pos));
    }
    advance();
}

int32_t Parser::getLine() {
    if(curr_pos >= tokens.size()) return 0;
    return tokens[curr_pos].line;
}

int32_t Parser::getColumn() {
    if(curr_pos >= tokens.size()) return 0;
    return tokens[curr_pos].column;
}

Expr* Parser::parseExpression() {
    return parseAssignment();
}

Expr* Parser::parseAssignment() {
    int32_t line = getLine();
    int32_t column = getColumn();
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
    return new BinaryExpr(left, op, right, line, column);
}

Expr* Parser::parseLogicalOr() {
    Expr* left = parseLogicalAnd();
    while(match(Token::OR)) {
        int32_t line = getLine();
        int32_t column = getColumn();
        Token op = Token::OR;
        Expr* right = parseLogicalAnd();
        left = new BinaryExpr(left, op, right, line, column);
    }
    return left;
}

Expr* Parser::parseLogicalAnd() {
    Expr* left = parseBitwiseOr();
    while(match(Token::AND)) {
        int32_t line = getLine();
        int32_t column = getColumn();
        Token op = Token::AND;
        Expr* right = parseBitwiseOr();
        left = new BinaryExpr(left, op, right, line, column);
    }
    return left;
}

Expr* Parser::parseBitwiseOr() {
    Expr* left = parseBitwiseXor();
    while(match(Token::BIT_OR)) {
        int32_t line = getLine();
        int32_t column = getColumn();
        Token op = Token::BIT_OR;
        Expr* right = parseBitwiseXor();
        left = new BinaryExpr(left, op, right, line, column);
    }
    return left;
}

Expr* Parser::parseBitwiseXor() {
    Expr* left = parseBitwiseAnd();
    while(match(Token::BIT_XOR)) {
        int32_t line = getLine();
        int32_t column = getColumn();
        Token op = Token::BIT_XOR;
        Expr* right = parseBitwiseAnd();
        left = new BinaryExpr(left, op, right, line, column);
    }
    return left;
}

Expr* Parser::parseBitwiseAnd() {
    Expr* left = parseEquality();
    while(match(Token::BIT_AND)) {
        int32_t line = getLine();
        int32_t column = getColumn();
        Token op = Token::BIT_AND;
        Expr* right = parseEquality();
        left = new BinaryExpr(left, op, right, line, column);
    }
    return left;
}

Expr* Parser::parseEquality() {
    Expr* left = parseRelational();
    while(true) {
        int32_t line = getLine();
        int32_t column = getColumn();
        Token op;
        if(match(Token::EQ)) {
            op = Token::EQ;
        } else if(match(Token::NE)) {
            op = Token::NE;
        } else {
            break;
        }
        Expr* right = parseRelational();
        left = new BinaryExpr(left, op, right, line, column);
    }
    return left;
}

Expr* Parser::parseRelational() {
    Expr* left = parseShift();
    while(true) {
        int32_t line = getLine();
        int32_t column = getColumn();
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
        left = new BinaryExpr(left, op, right, line, column);
    }
    return left;
}

Expr* Parser::parseShift() {
    Expr* left = parseAdditive();
    while(true) {
        int32_t line = getLine();
        int32_t column = getColumn();
        Token op;
        if(match(Token::SHL)) {
            op = Token::SHL;
        } else if(match(Token::SHR)) {
            op = Token::SHR;
        } else {
            break;
        }
        Expr* right = parseAdditive();
        left = new BinaryExpr(left, op, right, line, column);
    }
    return left;
}

Expr* Parser::parseAdditive() {
    Expr* left = parseMultiplicative();
    while(true) {
        int32_t line = getLine();
        int32_t column = getColumn();
        Token op;
        if(match(Token::PLUS)) {
            op = Token::PLUS;
        } else if(match(Token::MINUS)) {
            op = Token::MINUS;
        } else {
            break;
        }
        Expr* right = parseMultiplicative();
        left = new BinaryExpr(left, op, right, line, column);
    }
    return left;
}

Expr* Parser::parseMultiplicative() {
    Expr* left = parseUnary();
    while(true) {
        int32_t line = getLine();
        int32_t column = getColumn();
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
        left = new BinaryExpr(left, op, right, line, column);
    }
    return left;
}

Expr* Parser::parseUnary() {
    int32_t line = getLine();
    int32_t column = getColumn();
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
    return new UnaryExpr(op, operand, line, column);
}

Expr* Parser::parsePostfix() {
    Expr* left = parsePrimary();
    while(true) {
        int32_t line = getLine();
        int32_t column = getColumn();
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
            left = new CallExpr(funcName, args, line, column);
        } else if(match(Token::LBRACKET)) {
            Expr* index = parseExpression();
            expect(Token::RBRACKET);
            left = new ArrayAccessExpr(left, index, line, column);
        } else {
            break;
        }
    }
    return left;
}

Expr* Parser::parsePrimary() {
    int32_t line = getLine();
    int32_t column = getColumn();
    if(match(Token::NUMBER) || match(Token::STRING) || match(Token::CHAR) || 
       match(Token::TRUE) || match(Token::FALSE) || match(Token::NULL_KW)) {
        Token type = tokens[curr_pos - 1].type;
        std::string value = tokens[curr_pos - 1].value;
        return new LiteralExpr(type, value, line, column);
    } else if(match(Token::IDENTIFIER)) {
        std::string name = tokens[curr_pos - 1].value;
        return new VariableExpr(name, line, column);
    } else if(match(Token::LPAREN)) {
        Expr* expr = parseExpression();
        expect(Token::RPAREN);
        return expr;
    } else if(match(Token::IN)) {
        expect(Token::LPAREN);
        std::string varName = tokens[curr_pos].value;
        expect(Token::IDENTIFIER);
        expect(Token::RPAREN);
        return new InExpr(varName, line, column);
    } else if(match(Token::OUT)) {
        expect(Token::LPAREN);
        std::string varName = tokens[curr_pos].value;
        expect(Token::IDENTIFIER);
        expect(Token::RPAREN);
        return new OutExpr(varName, line, column);
    }
    else {
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
        case Token::LOOP:
            return parseLoopStmt();
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
    int32_t line = getLine();
    int32_t column = getColumn();
    Token type = advance();
    if(match(Token::LBRACKET)) {
        expect(Token::RBRACKET);
        switch (type) {
            case Token::S8:
                type = Token::S8_ARRAY;
                break;
            case Token::S16:
                type = Token::S16_ARRAY;
                break;
            case Token::S32:
                type = Token::S32_ARRAY;
                break;
            case Token::US8:
                type = Token::US8_ARRAY;
                break;
            case Token::US16:
                type = Token::US16_ARRAY;
                break;
            case Token::US32:
                type = Token::US32_ARRAY;
                break;
            case Token::BOOL:
                type = Token::BOOL_ARRAY;
                break;
            default:
                break;
        }
    }
    std::string variableName = tokens[curr_pos].value;
    expect(Token::IDENTIFIER);
    Expr* initializer = nullptr;
    if(match(Token::ASSIGN)) {
        initializer = parseExpression();
    }
    expect(Token::SEMICOLON);
    return new VarDeclStmt(type, variableName, initializer, line, column);
}

Stmt* Parser::parseIfStmt() {
    int32_t line = getLine();
    int32_t column = getColumn();
    expect(Token::IF);
    expect(Token::LPAREN);
    Expr* condition = parseExpression();
    expect(Token::RPAREN);
    Stmt* blockThen = parseStatement();
    Stmt* blockElse = nullptr;
    if(match(Token::ELSE)) {
        blockElse = parseStatement();
    }
    return new IfStmt(condition, blockThen, blockElse, line, column);
}

Stmt* Parser::parseLoopStmt() {
    int32_t line = getLine();
    int32_t column = getColumn();
    expect(Token::LOOP);
    expect(Token::LPAREN);
    
    int semicolonCount = 0;
    int parenDepth = 1;
    size_t lookaheadPos = curr_pos;
    
    while (parenDepth > 0 && lookaheadPos < tokens.size()) {
        if (tokens[lookaheadPos].type == Token::LPAREN) {
            parenDepth++;
        } else if (tokens[lookaheadPos].type == Token::RPAREN) {
            parenDepth--;
        } else if (tokens[lookaheadPos].type == Token::SEMICOLON && parenDepth == 1) {
            semicolonCount++;
        }
        lookaheadPos++;
    }
    
    Stmt* init = nullptr;
    Expr* condition = nullptr;
    Expr* increment = nullptr;
    
    if (semicolonCount == 2) {
        if (peek() != Token::SEMICOLON) {
            init = parseStatement();
        } else {
            expect(Token::SEMICOLON);
        }
        
        if (peek() != Token::SEMICOLON) {
            condition = parseExpression();
        }
        expect(Token::SEMICOLON);
        
        if (peek() != Token::RPAREN) {
            increment = parseExpression();
        }
        
    } else if (semicolonCount == 0) {
        condition = parseExpression();
        
    } else {
        throw std::runtime_error("loop statement must have either 0 or 2 semicolons");
    }
    
    expect(Token::RPAREN);
    
    Stmt* body = parseStatement();
    
    return new LoopStmt(init, condition, increment, body, line, column);
}

Stmt* Parser::parseReturnStmt() {
    int32_t line = getLine();
    int32_t column = getColumn();
    expect(Token::RETURN);
    Expr* returnValue = nullptr;
    if(!check(Token::SEMICOLON)) {
        returnValue = parseExpression();
    }
    expect(Token::SEMICOLON);
    return new ReturnStmt(returnValue, line, column);
}

Stmt* Parser::parseSendStmt() {
    int32_t line = getLine();
    int32_t column = getColumn();
    expect(Token::SEND);
    expect(Token::LPAREN);
    std::string variableName = tokens[curr_pos].value;
    expect(Token::IDENTIFIER);
    expect(Token::RARROW);
    std::string functionName = tokens[curr_pos].value;
    expect(Token::IDENTIFIER);
    expect(Token::RPAREN);
    expect(Token::SEMICOLON);
    return new SendStmt(variableName, functionName, line, column);
}

Stmt* Parser::parseRecvStmt() {
    int32_t line = getLine();
    int32_t column = getColumn();
    expect(Token::RECV);
    expect(Token::LPAREN);
    std::string srcFunction = tokens[curr_pos].value;
    expect(Token::IDENTIFIER);
    expect(Token::RARROW);
    std::string variableName = tokens[curr_pos].value;
    expect(Token::IDENTIFIER);
    expect(Token::RPAREN);
    expect(Token::SEMICOLON);
    return new RecvStmt(variableName, srcFunction, line, column);
}

Stmt* Parser::parseExprStmt() {
    int32_t line = getLine();
    int32_t column = getColumn();
    Expr* expression = parseExpression();
    expect(Token::SEMICOLON);
    return new ExprStmt(expression, line, column);
}

Stmt* Parser::parseBlock() {
    int32_t line = getLine();
    int32_t column = getColumn();
    expect(Token::LBRACE);
    std::vector<Stmt*> statements;
    while(!check(Token::RBRACE)) {
        statements.push_back(parseStatement());
    }
    expect(Token::RBRACE);
    return new BlockStmt(statements, line, column);
}

ImportDecl* Parser::parseImport() {
    int32_t line = getLine();
    int32_t column = getColumn();

    expect(Token::IMPORT);

    if(!check(Token::IDENTIFIER)) {
        throw std::runtime_error("Expected module name after 'import'");
    }

    std::string moduleName = tokens[curr_pos].value;
    advance();

    std::vector<std::string> symbols;
    if(match(Token::LBRACE)) {
        do { 
            if(!check(Token::IDENTIFIER)) {
                throw std::runtime_error("Expected symbol name in import");
            }
            symbols.push_back(tokens[curr_pos].value);
            advance();
        } while(match(Token::COMMA));

        expect(Token::RBRACE);
    }

    expect(Token::SEMICOLON);

    return new ImportDecl(moduleName, symbols, line, column);
}
FunctionDecl* Parser::parseFunction() {
    int32_t line = getLine();
    int32_t column = getColumn();
    if(isAtEnd()) {
        throw std::runtime_error("Expected function but got EOF");
    }
    std::string name = tokens[curr_pos].value;
    expect(Token::IDENTIFIER);
    std::vector<Parameter> parameters;
    expect(Token::LPAREN);
    if(!check(Token::RPAREN)) {
        while(true) {
            Token parameterType = advance();
            if(match(Token::LBRACKET)) {
                expect(Token::RBRACKET);
                switch(parameterType) {
                    case Token::S8:
                        parameterType = Token::S8_ARRAY;
                        break;
                    case Token::S16:
                        parameterType = Token::S16_ARRAY;
                        break;
                    case Token::S32:
                        parameterType = Token::S32_ARRAY;
                        break;
                    case Token::US8:
                        parameterType = Token::US8_ARRAY;
                        break;
                    case Token::US16:
                        parameterType = Token::US16_ARRAY;
                        break;
                    case Token::US32:
                        parameterType = Token::US32_ARRAY;
                        break;
                    case Token::BOOL:
                        parameterType = Token::BOOL_ARRAY;
                        break;
                    default:
                        break;
                }
            }
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

    if(match(Token::LBRACKET)) {
        expect(Token::RBRACKET);
        switch(returnType) {
            case Token::S8:
                returnType = Token::S8_ARRAY;
                break;
            case Token::S16:
                returnType = Token::S16_ARRAY;
                break;
            case Token::S32:
                returnType = Token::S32_ARRAY;
                break;
            case Token::US8:
                returnType = Token::US8_ARRAY;
                break;
            case Token::US16:
                returnType = Token::US16_ARRAY;
                break;
            case Token::US32:
                returnType = Token::US32_ARRAY;
                break;
            case Token::BOOL:
                returnType = Token::BOOL_ARRAY;
                break;
            default:
                break;
            }
        }
    
    Stmt* body = parseBlock();
    return new FunctionDecl(name, parameters, returnType, body, line, column);
}