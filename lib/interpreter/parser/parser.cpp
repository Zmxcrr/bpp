#include "parser.h"
#include "../expression/expression.h"
#include <iostream>
#include <algorithm>

namespace Interpreter {
     Token Parser::currentToken() const { return (pos < tokens.size()) ? tokens[pos] : tokens.back(); }

     const Token &Parser::previousToken() const {
        return (pos > 0 && pos <= tokens.size()) ? tokens[pos - 1] : tokens.front();
    }

     bool Parser::isAtEnd() const { return currentToken().type == TokenType::EndOfFile; }

    Token Parser::advance() {
        if (!isAtEnd()) pos++;
        return previousToken();
    }

    bool Parser::check(TokenType type) const {
        if (isAtEnd()) return false;
        return currentToken().type == type;
    }

    bool Parser::match(const std::vector<TokenType> &types) {
        return std::ranges::any_of(types, [this](TokenType type) {
            if (check(type)) {
                advance();
                return true;
            }
            return false;
        });
    }

     bool Parser::match(TokenType type) { return match(std::vector<TokenType>{type}); }

    void Parser::error(const Token &token, const std::string &message) {
        hadError = true;
        std::cerr << "[Строка " << token.line << ", столбец " << token.column << "] Ошибка парсинга";
        if (token.type == TokenType::EndOfFile) std::cerr << " в конце файла";
        else std::cerr << " у '" << token.text << "'";
        std::cerr << ": " << message << std::endl;
    }

    Token Parser::expect(TokenType type, const std::string &errorMessage) {
        if (check(type)) return advance();
        error(currentToken(), errorMessage);
        throw std::runtime_error("Ошибка синтаксиса: " + errorMessage);
    }

    void Parser::synchronize() {
        advance();
        while (!isAtEnd()) {
            if (previousToken().type == TokenType::KeywordEnd) return;
            switch (currentToken().type) {
                case TokenType::KeywordFunction:
                case TokenType::KeywordIf:
                case TokenType::KeywordWhile:
                case TokenType::KeywordFor:
                case TokenType::KeywordReturn:
                case TokenType::KeywordPrint:
                case TokenType::KeywordPrintln:
                case TokenType::KeywordBreak:
                case TokenType::KeywordContinue: return;
                default: break;
            }
            advance();
        }
    }

    std::unique_ptr<Stmt> Parser::parseDeclaration() {
        try { 
		return parseStatement(); 
	} catch (const std::runtime_error &) {
            synchronize();
            return nullptr;
        }
    }

    std::unique_ptr<Stmt> Parser::parseStatement() {
        if (match(TokenType::KeywordIf)) return parseIfStatement();
        if (match(TokenType::KeywordPrint)) return parsePrintStatement();
        if (match(TokenType::KeywordPrintln)) return parsePrintlnStatement();
        if (match(TokenType::KeywordReturn)) return parseReturnStatement();
        if (match(TokenType::KeywordWhile)) return parseWhileStatement();
        if (match(TokenType::KeywordFor)) return parseForStatement();
        if (match(TokenType::KeywordBreak)) return std::make_unique<BreakStmt>();
        if (match(TokenType::KeywordContinue)) return std::make_unique<ContinueStmt>();
        return parseExpressionStatement();
    }

    std::unique_ptr<Stmt> Parser::parsePrintStatement() {
        bool hasParen = match(TokenType::LParen);
        std::unique_ptr<Expr> expr = nullptr;
        if (!hasParen || !check(TokenType::RParen)) expr = parseLogicalOr();
        if (hasParen) expect(TokenType::RParen, "Ожидалась ')' после аргументов 'print'.");
        if (!expr) expr = std::make_unique<NilExpr>();
        return std::make_unique<PrintStmt>(std::move(expr));
    }

    std::unique_ptr<Stmt> Parser::parsePrintlnStatement() {
        bool hasParen = match(TokenType::LParen);
        std::unique_ptr<Expr> expr = nullptr;
        if (!hasParen || !check(TokenType::RParen)) expr = parseLogicalOr();
        if (hasParen) expect(TokenType::RParen, "Ожидалась ')' после аргументов 'println'.");
        if (!expr) expr = std::make_unique<NilExpr>();
        return std::make_unique<PrintlnStmt>(std::move(expr));
    }

    std::unique_ptr<Stmt> Parser::parseReturnStatement() {
        Token keyword = previousToken();
        std::unique_ptr<Expr> value = nullptr;
        if (!check(TokenType::KeywordEnd) && !isAtEnd()) value = parseLogicalOr();
        return std::make_unique<ReturnStmt>(std::move(value));
    }

    std::vector<std::unique_ptr<Stmt> > Parser::
    parseBlockBody(TokenType endKeyword, const std::string &blockName) {
        std::vector<std::unique_ptr<Stmt> > statements;
        while (!check(endKeyword) && !isAtEnd())
            if (std::unique_ptr<Stmt> decl = parseDeclaration()) statements.push_back(std::move(decl));
        expect(endKeyword,
               "Ожидалось '" + Token{endKeyword, "", 0, 0}.text + "' для завершения блока '" + blockName + "'");
        TokenType blockTypeKeyword = TokenType::Unknown;
        if (blockName == "if") blockTypeKeyword = TokenType::KeywordIf;
        else if (blockName == "while") blockTypeKeyword = TokenType::KeywordWhile;
        else if (blockName == "for") blockTypeKeyword = TokenType::KeywordFor;
        else if (blockName == "function") blockTypeKeyword = TokenType::KeywordFunction;
        match(blockTypeKeyword);
        return statements;
    }

    std::unique_ptr<Stmt> Parser::parseIfStatement() {
        auto condition = parseLogicalOr();
        expect(TokenType::KeywordThen, "Ожидалось 'then' после условия if.");
        std::vector<std::unique_ptr<Stmt> > thenBranch;
        std::vector<std::pair<std::unique_ptr<Expr>, std::vector<std::unique_ptr<Stmt> > > > elseIfBranches;
        std::vector<std::unique_ptr<Stmt> > elseBranch;
        while (!check(TokenType::KeywordElse) && !check(TokenType::KeywordEnd) && !isAtEnd()) {
            if (std::unique_ptr<Stmt> stmt = parseDeclaration()) thenBranch.push_back(std::move(stmt));
            if (hadError && !check(TokenType::KeywordElse) && !check(TokenType::KeywordEnd) && !isAtEnd()) break;
        }
        while (match(TokenType::KeywordElse)) {
            if (match(TokenType::KeywordIf)) {
                auto elseIfCondition = parseLogicalOr();
                expect(TokenType::KeywordThen, "Ожидалось 'then' после условия else if.");
                std::vector<std::unique_ptr<Stmt> > elseIfBody;
                while (!check(TokenType::KeywordElse) && !check(TokenType::KeywordEnd) && !isAtEnd()) {
                    if (std::unique_ptr<Stmt> stmt = parseDeclaration()) elseIfBody.push_back(std::move(stmt));
                    if (hadError && !check(TokenType::KeywordElse) && !check(TokenType::KeywordEnd) && !isAtEnd())
                        break
                                ;
                }
                elseIfBranches.emplace_back(std::move(elseIfCondition), std::move(elseIfBody));
            } else {
                while (!check(TokenType::KeywordEnd) && !isAtEnd()) {
                    if (std::unique_ptr<Stmt> stmt = parseDeclaration()) elseBranch.push_back(std::move(stmt));
                    if (hadError && !check(TokenType::KeywordEnd) && !isAtEnd()) break;
                }
                expect(TokenType::KeywordEnd, "Ожидалось 'end' для завершения if-оператора.");
                match(TokenType::KeywordIf);
                return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch),
                                                std::move(elseIfBranches), std::move(elseBranch));
                break;
            }
        }
        expect(TokenType::KeywordEnd, "Ожидалось 'end' для завершения if-оператора.");
        match(TokenType::KeywordIf);
        return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseIfBranches),
                                        std::move(elseBranch));
    }

    std::unique_ptr<Stmt> Parser::parseWhileStatement() {
        auto condition = parseLogicalOr();
        std::vector<std::unique_ptr<Stmt> > body = parseBlockBody(TokenType::KeywordEnd, "while");
        return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
    }

    std::unique_ptr<Stmt> Parser::parseForStatement() {
        Token var = expect(TokenType::Identifier, "Ожидалось имя переменной после 'for'.");
        expect(TokenType::KeywordIn, "Ожидалось 'in' после переменной цикла for.");
        auto iterable = parseLogicalOr();
        std::vector<std::unique_ptr<Stmt> > body = parseBlockBody(TokenType::KeywordEnd, "for");
        return std::make_unique<ForStmt>(var.text, std::move(iterable), std::move(body));
    }

    std::unique_ptr<Stmt> Parser::parseExpressionStatement() {
        auto expr = parseLogicalOr();
        if (match({
            TokenType::Equals, TokenType::PlusEquals, TokenType::MinusEquals,
            TokenType::StarEquals, TokenType::SlashEquals, TokenType::PercentEquals,
            TokenType::CaretEquals
        })) {
            Token opToken = previousToken();
            auto right = parseLogicalOr();
            if (auto varExpr = dynamic_cast<VariableExpr *>(expr.get())) {
                std::string varName = varExpr->name;
                if (opToken.type == TokenType::Equals)
                    return std::make_unique<AssignStmt>(
                        std::move(varName), std::move(right));
                else {
                    std::string op = opToken.text.substr(0, opToken.text.size() - 1);
                    return std::make_unique<CompoundAssignStmt>(std::move(varName), op, std::move(right));
                }
            } else if (auto idx = dynamic_cast<IndexExpr *>(expr.get())) {
                return std::make_unique<AssignIndexStmt>(
                    std::move(idx->base),
                    std::move(idx->index),
                    std::move(right)
                );
            } else if (auto sl = dynamic_cast<SliceExpr *>(expr.get())) {
                return std::make_unique<AssignSliceStmt>(
                    std::move(sl->base),
                    std::move(sl->start),
                    std::move(sl->end),
                    std::move(right)
                );
            } else {
                error(opToken, "Недопустимая цель для операции присваивания.");
                return nullptr;
            }
        }
        return std::make_unique<ExpressionStmt>(std::move(expr));
    }

    std::unique_ptr<Expr> Parser::parseLogicalOr() {
        auto left = parseLogicalAnd();
        while (match(TokenType::Or)) {
            Token op = previousToken();
            auto right = parseLogicalAnd();
            left = std::make_unique<BinaryExpr>(std::move(left), op.text, std::move(right));
        }
        return left;
    }

    std::unique_ptr<Expr> Parser::parseLogicalAnd() {
        auto left = parseEquality();
        while (match(TokenType::And)) {
            Token op = previousToken();
            auto right = parseEquality();
            left = std::make_unique<BinaryExpr>(std::move(left), op.text, std::move(right));
        }
        return left;
    }

    std::unique_ptr<Expr> Parser::parseEquality() {
        auto left = parseComparison();
        while (match({TokenType::EqualEqual, TokenType::NotEqual})) {
            Token op = previousToken();
            auto right = parseComparison();
            left = std::make_unique<BinaryExpr>(std::move(left), op.text, std::move(right));
        }
        return left;
    }

    std::unique_ptr<Expr> Parser::parseComparison() {
        auto left = parseTerm();
        while (match({TokenType::Greater, TokenType::GreaterEqual, TokenType::Less, TokenType::LessEqual})) {
            Token op = previousToken();
            auto right = parseTerm();
            left = std::make_unique<BinaryExpr>(std::move(left), op.text, std::move(right));
        }
        return left;
    }

    std::unique_ptr<Expr> Parser::parseTerm() {
        auto left = parseFactor();
        while (match({TokenType::Minus, TokenType::Plus})) {
            Token op = previousToken();
            auto right = parseFactor();
            left = std::make_unique<BinaryExpr>(std::move(left), op.text, std::move(right));
        }
        return left;
    }

    std::unique_ptr<Expr> Parser::parseFactor() {
        auto left = parseExponent();
        while (match({TokenType::Slash, TokenType::Star, TokenType::Percent})) {
            Token op = previousToken();
            auto right = parseExponent();
            left = std::make_unique<BinaryExpr>(std::move(left), op.text, std::move(right));
        }
        return left;
    }

    std::unique_ptr<Expr> Parser::parseExponent() {
        auto left = parseUnary();
        if (match(TokenType::Caret)) {
            Token op = previousToken();
            auto right = parseExponent();
            left = std::make_unique<BinaryExpr>(std::move(left), op.text, std::move(right));
        }
        return left;
    }

    std::unique_ptr<Expr> Parser::parseUnary() {
        if (match({TokenType::Not, TokenType::Minus, TokenType::Plus})) {
            Token op = previousToken();
            auto operand = parseUnary();
            return std::make_unique<UnaryExpr>(op.text, std::move(operand));
        }
        return parsePostfix();
    }

    std::unique_ptr<Expr> Parser::parsePostfix() {
        auto expr = parsePrimary();
        while (true) {
            if (match(TokenType::LParen)) {
                std::vector<std::unique_ptr<Expr> > args;
                if (!check(TokenType::RParen)) {
                    do {
                        if (check(TokenType::RParen)) break;
                        args.push_back(parseLogicalOr());
                    } while (match(TokenType::Comma));
                }
                expect(TokenType::RParen, "Ожидалась ')' после аргументов функции.");
                expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
            } else if (match(TokenType::LBracket)) {
                std::unique_ptr<Expr> start = nullptr, end = nullptr;
                bool isSlice = false;
                if (match(TokenType::Colon)) {
                    isSlice = true;
                    if (!check(TokenType::RBracket)) end = parseLogicalOr();
                } else {
                    start = parseLogicalOr();
                    if (match(TokenType::Colon)) {
                        isSlice = true;
                        if (!check(TokenType::RBracket)) end = parseLogicalOr();
                    } else isSlice = false;
                }
                expect(TokenType::RBracket, "Ожидалась ']' после индекса или среза.");
                if (isSlice) expr = std::make_unique<SliceExpr>(std::move(expr), std::move(start), std::move(end));
                else expr = std::make_unique<IndexExpr>(std::move(expr), std::move(start));
            } else { break; }
        }
        return expr;
    }

    std::unique_ptr<Expr> Parser::parsePrimary() {
        if (match(TokenType::KeywordFalse)) return std::make_unique<BoolExpr>(false);
        if (match(TokenType::KeywordTrue)) return std::make_unique<BoolExpr>(true);
        if (match(TokenType::KeywordNil)) return std::make_unique<NilExpr>();
        if (match(TokenType::Number)) {
            try { return std::make_unique<NumberExpr>(std::stod(previousToken().text)); } catch (const
                std::exception &) {
                error(previousToken(), "Неверный формат числа.");
                return std::make_unique<NilExpr>();
            }
        }
        if (match(TokenType::String)) return std::make_unique<StringExpr>(previousToken().text);
        if (match(TokenType::Identifier)) return std::make_unique<VariableExpr>(previousToken().text);
        if (match(TokenType::LParen)) {
            auto expr = parseLogicalOr();
            expect(TokenType::RParen, "Ожидалась ')' после выражения в скобках.");
            return expr;
        }
        if (match(TokenType::KeywordFunction)) return parseFunctionLiteral();
        if (match(TokenType::LBracket)) return parseArrayLiteral();
        error(currentToken(),
              "Ожидалось выражение (число, строка, переменная, 'true', 'false', 'nil', '(', '[', 'function').");
        return std::make_unique<NilExpr>();
    }

    std::unique_ptr<Expr> Parser::parseFunctionLiteral() {
        Token keyword = previousToken();
        expect(TokenType::LParen, "Ожидалась '(' после 'function' для параметров.");
        std::vector<std::string> params;
        if (!check(TokenType::RParen)) {
            do {
                if (check(TokenType::RParen)) break;
                Token p = expect(TokenType::Identifier, "Ожидалось имя параметра.");
                params.push_back(p.text);
            } while (match(TokenType::Comma));
        }
        expect(TokenType::RParen, "Ожидалась ')' после параметров функции.");
        std::vector<std::unique_ptr<Stmt> > body = parseBlockBody(TokenType::KeywordEnd, "function");
        return std::make_unique<FunctionExpr>(
            std::move(params),
            std::move(body),
            keyword.line,
            keyword.column
        );
    }

    std::unique_ptr<Expr> Parser::parseArrayLiteral() {
        Token bracket = previousToken();
        std::vector<std::unique_ptr<Expr> > elements;
        if (!check(TokenType::RBracket)) {
            do {
                if (check(TokenType::RBracket)) break;
                elements.push_back(parseLogicalOr());
            } while (match(TokenType::Comma));
        }
        expect(TokenType::RBracket, "Ожидалась ']' для завершения литерала массива.");
        return std::make_unique<ArrayExpr>(std::move(elements));
    }

     Parser::Parser(const std::vector<Token> &ts): tokens(ts), pos(0), hadError(false) {}

    std::vector<std::unique_ptr<Stmt> > Parser::parse() {
        hadError = false;
        std::vector<std::unique_ptr<Stmt> > statements;
        while (!isAtEnd())
            if (std::unique_ptr<Stmt> decl = parseDeclaration()) statements.push_back(std::move(decl));
        return statements;
    }

     bool Parser::parsingFailed() const { return hadError; }

} //Interpreter
