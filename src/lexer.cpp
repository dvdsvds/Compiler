#include "lexer.hpp"

Lexer::Lexer(const std::string& src) : source(src), pos(0), line(1), column(0) {
    keywords["import"] = Token::IMPORT;
    keywords["print"] = Token::PRINT;
    keywords["struct"] = Token::STRUCT;
    keywords["enum"] = Token::ENUM;
    keywords["if"] = Token::IF;
    keywords["else"] = Token::ELSE;
    keywords["loop"] = Token::LOOP;
    keywords["in"] = Token::IN;
    keywords["out"] = Token::OUT;
    keywords["return"] = Token::RETURN;
    keywords["break"] = Token::BREAK;
    keywords["continue"] = Token::CONTINUE;
    keywords["switch"] = Token::SWITCH;
    keywords["case"] = Token::CASE;
    keywords["default"] = Token::DEFAULT;
    keywords["asm"] = Token::ASM;
    keywords["send"] = Token::SEND;
    keywords["recv"] = Token::RECV;
    keywords["null"] = Token::NULL_KW;
    keywords["as"] = Token::AS;
    keywords["s8"] = Token::S8;
    keywords["s16"] = Token::S16;
    keywords["s32"] = Token::S32;
    keywords["us8"] = Token::US8;
    keywords["us16"] = Token::US16;
    keywords["us32"] = Token::US32;
    keywords["ps8"] = Token::PS8;
    keywords["ps16"] = Token::PS16;
    keywords["ps32"] = Token::PS32;
    keywords["pus8"] = Token::PUS8;
    keywords["pus16"] = Token::PUS16;
    keywords["pus32"] = Token::PUS32;
    keywords["bool"] = Token::BOOL;
    keywords["true"] = Token::TRUE;
    keywords["false"] = Token::FALSE;
    keywords["void"] = Token::VOID;
}

char Lexer::current_char() {
    if(pos >= source.length()) {
        return '\0';
    }
    return source[pos];
}
char Lexer::peek_char() {
    if(pos + 1 >= source.length()) {
        return '\0';
    }
    return source[pos + 1];
}
void Lexer::advance() {
    if(current_char() == '\n') {
        line++;
        column = 0;
    } else {
        column++;
    }
    pos++;
}
void Lexer::skip_whitespace() {
    while(current_char() == ' ' ||
          current_char() == '\t' ||
          current_char() == '\n' ||
          current_char() == '\r') {
            advance();
          }
}
void Lexer::skip_comment() {
    if(current_char() == '/' && peek_char() == '/') {
        while(current_char() != '\n' && current_char() != '\0') {
            advance();
        }
    }

    else if(current_char() == '/' && peek_char() == '*') {
        advance();
        advance();

        while(!(current_char() == '*' && peek_char() == '/') && current_char() != '\0') {
                    advance();
        }

        if(current_char() == '*') {
            advance();
            advance();
        }
    }
}
TokenData Lexer::read_number() {
    int32_t start_col = column;
    std::string value;

    if(current_char() == '0' && peek_char() == 'x') {
        value += current_char();
        advance();
        value += current_char();
        advance();
    
        while(isxdigit(current_char())) {
            value += current_char();
            advance();
        }
    } else {
        while(isdigit(current_char())) {
            value += current_char();
            advance();
        }
    }
    return TokenData{Token::NUMBER, value, line, start_col};
}
TokenData Lexer::read_string() {
    int32_t start_col = column;
    std::string value;

    if(current_char() == '\"') {
        advance();
    }

    while(current_char() != '\"' && current_char() != '\0' && current_char() != '\n') {
        value += current_char();
        advance();
    }

    if(current_char() == '\"') {
        advance();
    }

    return TokenData{Token::STRING, value, line, start_col};
}
TokenData Lexer::read_char() {
    int32_t start_col = column;
    std::string value;

    if(current_char() == '\'') {
        advance();
    }

    while(current_char() != '\'' && current_char() != '\0' && current_char() != '\n') {
        value += current_char();
        advance();
    }

    if(current_char() == '\'') {
        advance();
    }

    return TokenData{Token::CHAR, value, line, start_col};
}
TokenData Lexer::read_identifier() {
    int32_t start_col = column;
    std::string value;

    while(isalnum(current_char()) || current_char() == '_') {
        value += current_char();
        advance();
    }

    auto it = keywords.find(value);
    if(it != keywords.end()) {
        return TokenData{it->second, value, line, start_col};
    }

    return TokenData{Token::IDENTIFIER, value, line, start_col};
}
TokenData Lexer::next_token() {
    skip_whitespace();
    skip_comment();

    if(current_char() == '\0') {
        return TokenData{Token::EOF_TOKEN, "", line, column};
    }

    if(isdigit(current_char())) {
        return read_number();
    } else if(current_char() == '\"') {
        return read_string();
    } else if(current_char() == '\'') {
        return read_char();
    } else if(isalpha(current_char()) || current_char() == '_') {
        return read_identifier();
    }

    int32_t start_col = column;

    switch(current_char()) {
        case '+':
            advance();
            if (current_char() == '+') {
                advance();
                return TokenData{Token::INC, "++", line, start_col};
            } else if (current_char() == '=') {
                advance();
                return TokenData{Token::PLUS_EQ, "+=", line, start_col};
            } else {
                return TokenData{Token::PLUS, "+", line, start_col};
            }
            
        case '-':
            advance();
            if (current_char() == '-') {
                advance();
                return TokenData{Token::DEC, "--", line, start_col};
            } else if (current_char() == '=') {
                advance();
                return TokenData{Token::MINUS_EQ, "-=", line, start_col};
            } else if (current_char() == '>') {
                advance();
                return TokenData{Token::RARROW, "->", line, start_col};
            } else {
                return TokenData{Token::MINUS, "-", line, start_col};
            }
            
        case '*':
            advance();
            if (current_char() == '=') {
                advance();
                return TokenData{Token::STAR_EQ, "*=", line, start_col};
            } else {
                return TokenData{Token::STAR, "*", line, start_col};
            }
            
        case '/':
            advance();
            if (current_char() == '=') {
                advance();
                return TokenData{Token::SLASH_EQ, "/=", line, start_col};
            } else {
                return TokenData{Token::SLASH, "/", line, start_col};
            }
            
        case '%':
            advance();
            if (current_char() == '=') {
                advance();
                return TokenData{Token::PERCENT_EQ, "%=", line, start_col};
            } else {
                return TokenData{Token::PERCENT, "%", line, start_col};
            }
            
        case '=':
            advance();
            if (current_char() == '=') {
                advance();
                return TokenData{Token::EQ, "==", line, start_col};
            } else if (current_char() == '>') {
                advance();
                return TokenData{Token::ARROW, "=>", line, start_col};
            } else {
                return TokenData{Token::ASSIGN, "=", line, start_col};
            }
            
        case '!':
            advance();
            if (current_char() == '=') {
                advance();
                return TokenData{Token::NE, "!=", line, start_col};
            } else {
                return TokenData{Token::NOT, "!", line, start_col};
            }
            
        case '<':
            advance();
            if (current_char() == '=') {
                advance();
                return TokenData{Token::LE, "<=", line, start_col};
            } else if (current_char() == '<') {
                advance();
                if (current_char() == '=') {
                    advance();
                    return TokenData{Token::SHL_EQ, "<<=", line, start_col};
                } else {
                    return TokenData{Token::SHL, "<<", line, start_col};
                }
            } else if (current_char() == '-') {
                advance();
                return TokenData{Token::LARROW, "<-", line, start_col};
            } else {
                return TokenData{Token::LT, "<", line, start_col};
            }
            
        case '>':
            advance();
            if (current_char() == '=') {
                advance();
                return TokenData{Token::GE, ">=", line, start_col};
            } else if (current_char() == '>') {
                advance();
                if (current_char() == '=') {
                    advance();
                    return TokenData{Token::SHR_EQ, ">>=", line, start_col};
                } else {
                    return TokenData{Token::SHR, ">>", line, start_col};
                }
            } else {
                return TokenData{Token::GT, ">", line, start_col};
            }
            
        case '&':
            advance();
            if (current_char() == '&') {
                advance();
                return TokenData{Token::AND, "&&", line, start_col};
            } else if (current_char() == '=') {
                advance();
                return TokenData{Token::AND_EQ, "&=", line, start_col};
            } else {
                return TokenData{Token::BIT_AND, "&", line, start_col};
            }
            
        case '|':
            advance();
            if (current_char() == '|') {
                advance();
                return TokenData{Token::OR, "||", line, start_col};
            } else if (current_char() == '=') {
                advance();
                return TokenData{Token::OR_EQ, "|=", line, start_col};
            } else {
                return TokenData{Token::BIT_OR, "|", line, start_col};
            }
            
        case '^':
            advance();
            if (current_char() == '=') {
                advance();
                return TokenData{Token::XOR_EQ, "^=", line, start_col};
            } else {
                return TokenData{Token::BIT_XOR, "^", line, start_col};
            }
            
        case '~':
            advance();
            return TokenData{Token::BIT_NOT, "~", line, start_col};
            
        case '(':
            advance();
            return TokenData{Token::LPAREN, "(", line, start_col};
            
        case ')':
            advance();
            return TokenData{Token::RPAREN, ")", line, start_col};
            
        case '{':
            advance();
            return TokenData{Token::LBRACE, "{", line, start_col};
            
        case '}':
            advance();
            return TokenData{Token::RBRACE, "}", line, start_col};
            
        case '[':
            advance();
            return TokenData{Token::LBRACKET, "[", line, start_col};
            
        case ']':
            advance();
            return TokenData{Token::RBRACKET, "]", line, start_col};
            
        case ';':
            advance();
            return TokenData{Token::SEMICOLON, ";", line, start_col};
            
        case ':':
            advance();
            return TokenData{Token::COLON, ":", line, start_col};
            
        case ',':
            advance();
            return TokenData{Token::COMMA, ",", line, start_col};
            
        case '.':
            advance();
            return TokenData{Token::DOT, ".", line, start_col};
            
        case '#':
            advance();
            return TokenData{Token::HASH, "#", line, start_col};
            
        default:
            advance();
            return TokenData{Token::INVALID, std::string(1, current_char()), line, start_col};
    }
}
std::vector<TokenData> Lexer::tokenize() {
    std::vector<TokenData> tokens;

    while(current_char() != '\0') {
        tokens.push_back(next_token());
    }

    return tokens;
}