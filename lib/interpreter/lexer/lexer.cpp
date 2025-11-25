#include "lexer.h"
#include <map>

namespace Interpreter {
     char Lexer::peek(size_t offset) const { return (pos + offset) < input.size() ? input[pos + offset] : '\0'; }

    char Lexer::get() {
        if (pos < input.size()) {
            char c = input[pos++];
            if (c == '\n') {
                line++;
                column = 1;
            } else { column++; }
            return c;
        }
        return '\0';
    }

    void Lexer::skipWhitespaceAndComments() {
        while (true) {
            char current = peek();
            if (std::isspace(current)) {
                get();
                continue;
            }
            if (current == '/' && peek(1) == '/') {
                while (peek() != '\n' && peek() != '\0') get();
                continue;
            } else break;
        }
    }

     Token Lexer::makeToken(TokenType type, const std::string &text, int tokenStartColumn) const {
        return Token{type, text, line, tokenStartColumn};
    }

     Token Lexer::makeToken(TokenType type, char c, int tokenStartColumn) const {
        return makeToken(type, std::string(1, c), tokenStartColumn);
    }

     Lexer::Lexer(std::string input): input(std::move(input)), pos(0), line(1), column(1) {}

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
        while (true) {
            skipWhitespaceAndComments();
            int tokenStartColumn = column;
            char current = peek();
            if (current == '\0') {
                tokens.push_back(makeToken(TokenType::EndOfFile, "", tokenStartColumn));
                break;
            }
            if (current == '"') {
                get();
                std::string str;
                int stringStartCol = column;
                while (true) {
                    char c = peek();
                    if (c == '\0') {
                        tokens.push_back(makeToken(TokenType::Unknown, input.substr(pos - str.length() - 1),
                                                   stringStartCol - 1));
                        goto end_tokenize;
                    }
                    if (c == '"') {
                        get();
                        break;
                    }
                    if (c == '\\') {
                        get();
                        switch (char next = get()) {
                            case '"': str.push_back('"');
                                break;
                            case '\\': str.push_back('\\');
                                break;
                            case 'n': str.push_back('\n');
                                break;
                            case 't': str.push_back('\t');
                                break;
                            case 'r': str.push_back('\r');
                                break;
                            default: str.push_back('\\');
                                str.push_back(next);
                                break;
                        }
                    } else str.push_back(get());
                }
                tokens.push_back(makeToken(TokenType::String, str, tokenStartColumn));
                continue;
            }
            if (std::isdigit(current) || (current == '.' && std::isdigit(peek(1)))) {
                std::string num_str;
                int numStartCol = column;
                if (current == '.') {
                    num_str += get();
                    while (std::isdigit(peek())) num_str += get();
                } else {
                    num_str += get();
                    while (std::isdigit(peek())) num_str += get();
                    if (peek() == '.') {
                        num_str += get();
                        while (std::isdigit(peek())) num_str += get();
                    }
                }

                if (peek() == 'e' || peek() == 'E') {
                    num_str += get();
                    if (peek() == '+' || peek() == '-') num_str += get();
                    if (!std::isdigit(peek())) {
                        tokens.push_back(makeToken(TokenType::Unknown, num_str, numStartCol));
                        goto end_tokenize;
                    }
                    while (std::isdigit(peek())) num_str += get();
                }
                tokens.push_back(makeToken(TokenType::Number, num_str, tokenStartColumn));
                continue;
            }
            if (std::isalpha(current) || current == '_') {
                std::string word;
                word += get();
                while (std::isalnum(peek()) || peek() == '_') word += get();
                static const std::map<std::string, TokenType> keywords = {
                    {"function", TokenType::KeywordFunction}, {"return", TokenType::KeywordReturn},
                    {"end", TokenType::KeywordEnd}, {"print", TokenType::KeywordPrint},
                    {"println", TokenType::KeywordPrintln}, {"nil", TokenType::KeywordNil},
                    {"if", TokenType::KeywordIf}, {"then", TokenType::KeywordThen},
                    {"else", TokenType::KeywordElse}, {"true", TokenType::KeywordTrue},
                    {"false", TokenType::KeywordFalse}, {"for", TokenType::KeywordFor},
                    {"in", TokenType::KeywordIn}, {"while", TokenType::KeywordWhile},
                    {"and", TokenType::And}, {"or", TokenType::Or}, {"not", TokenType::Not},
                    {"break", TokenType::KeywordBreak}, {"continue", TokenType::KeywordContinue}
                };
                auto it = keywords.find(word);
                TokenType type = (it != keywords.end()) ? it->second : TokenType::Identifier;
                tokens.push_back(makeToken(type, word, tokenStartColumn));
                continue;
            }

#define SINGLE_CHAR_TOKEN(char_literal, token_type) \
    case char_literal: tokens.push_back(makeToken(token_type, get(), tokenStartColumn)); continue;

#define TWO_CHAR_TOKEN(char1, char2, type1, type2, text1, text2) \
    case char1: \
        if (peek(1) == char2) { get(); get(); tokens.push_back(makeToken(type2, text2, tokenStartColumn)); } \
        else { get(); tokens.push_back(makeToken(type1, text1, tokenStartColumn)); } \
        continue;

            switch (current) {
                TWO_CHAR_TOKEN('+', '=', TokenType::Plus, TokenType::PlusEquals, "+", "+=")
                TWO_CHAR_TOKEN('-', '=', TokenType::Minus, TokenType::MinusEquals, "-", "-=")
                TWO_CHAR_TOKEN('*', '=', TokenType::Star, TokenType::StarEquals, "*", "*=")
                case '/':
                    if (peek(1) == '=') {
                        get();
                        get();
                        tokens.push_back(makeToken(TokenType::SlashEquals, "/=", tokenStartColumn));
                    } else {
                        get();
                        tokens.push_back(makeToken(TokenType::Slash, "/", tokenStartColumn));
                    }
                    continue;
                TWO_CHAR_TOKEN('%', '=', TokenType::Percent, TokenType::PercentEquals, "%", "%=")
                TWO_CHAR_TOKEN('^', '=', TokenType::Caret, TokenType::CaretEquals, "^", "^=")
                TWO_CHAR_TOKEN('=', '=', TokenType::Equals, TokenType::EqualEqual, "=", "==")
                TWO_CHAR_TOKEN('!', '=', TokenType::Unknown, TokenType::NotEqual, "!", "!=")

                TWO_CHAR_TOKEN('<', '=', TokenType::Less, TokenType::LessEqual, "<", "<=")
                TWO_CHAR_TOKEN('>', '=', TokenType::Greater, TokenType::GreaterEqual, ">", ">=")

                SINGLE_CHAR_TOKEN('(', TokenType::LParen)
                SINGLE_CHAR_TOKEN(')', TokenType::RParen)
                SINGLE_CHAR_TOKEN('[', TokenType::LBracket)
                SINGLE_CHAR_TOKEN(']', TokenType::RBracket)
                SINGLE_CHAR_TOKEN(',', TokenType::Comma)
                SINGLE_CHAR_TOKEN(':', TokenType::Colon)

                default:
                    tokens.push_back(makeToken(TokenType::Unknown, get(), tokenStartColumn));
                    goto end_tokenize;
            }
        }
    end_tokenize:
        return tokens;
    }
} //Interpreter