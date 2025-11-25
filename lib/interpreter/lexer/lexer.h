#pragma once

#include "token.h"
#include <vector>

namespace Interpreter {
    class Lexer {
        std::string input;
        size_t pos;
        int line;
        int column;

        [[nodiscard]] char peek(size_t offset = 0) const;

        char get();

        void skipWhitespaceAndComments();

        [[nodiscard]] Token makeToken(TokenType type, const std::string &text, int tokenStartColumn) const;

        [[nodiscard]] Token makeToken(TokenType type, char c, int tokenStartColumn) const;

    public:
        explicit Lexer(std::string input);

        std::vector<Token> tokenize();
    };
} //Interpreter