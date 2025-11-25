#pragma once

#include "token.h"
#include "statement.h"

namespace Interpreter {
    class Parser {
        std::vector<Token> tokens;
        size_t pos;
        bool hadError = false;

        [[nodiscard]] Token currentToken() const;

        [[nodiscard]] const Token &previousToken() const;

        [[nodiscard]] bool isAtEnd() const;

        Token advance();

        [[nodiscard]] bool check(TokenType type) const;

        bool match(const std::vector<TokenType> &types);

        bool match(TokenType type);

        void error(const Token &token, const std::string &message);

        Token expect(TokenType type, const std::string &errorMessage);

        void synchronize();

        std::unique_ptr<Stmt> parseDeclaration();

        std::unique_ptr<Stmt> parseStatement();

        std::unique_ptr<Stmt> parsePrintStatement();

        std::unique_ptr<Stmt> parsePrintlnStatement();

        std::unique_ptr<Stmt> parseReturnStatement();

        std::vector<std::unique_ptr<Stmt> > parseBlockBody(TokenType endKeyword, const std::string &blockName);

        std::unique_ptr<Stmt> parseIfStatement();

        std::unique_ptr<Stmt> parseWhileStatement();

        std::unique_ptr<Stmt> parseForStatement();

        std::unique_ptr<Stmt> parseExpressionStatement();

        std::unique_ptr<Expr> parseLogicalOr();

        std::unique_ptr<Expr> parseLogicalAnd();

        std::unique_ptr<Expr> parseEquality();

        std::unique_ptr<Expr> parseComparison();

        std::unique_ptr<Expr> parseTerm();

        std::unique_ptr<Expr> parseFactor();

        std::unique_ptr<Expr> parseExponent();

        std::unique_ptr<Expr> parseUnary();

        std::unique_ptr<Expr> parsePostfix();

        std::unique_ptr<Expr> parsePrimary();

        std::unique_ptr<Expr> parseFunctionLiteral();

        std::unique_ptr<Expr> parseArrayLiteral();

    public:
        explicit Parser(const std::vector<Token> &ts);

        std::vector<std::unique_ptr<Stmt> > parse();

        [[nodiscard]] bool parsingFailed() const;
    };

} //Interpreter