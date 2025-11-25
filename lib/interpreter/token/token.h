#pragma once

#include <string>

namespace Interpreter {
    enum class TokenType {
        Identifier, Number, String,
        Plus, Minus, Star, Slash, Percent, Caret,
        Equals, PlusEquals, MinusEquals, StarEquals, SlashEquals, PercentEquals, CaretEquals,
        EqualEqual, NotEqual, Less, Greater, LessEqual, GreaterEqual,
        And, Or, Not,
        Comma, LParen, RParen, LBracket, RBracket, Colon,
        KeywordFunction, KeywordReturn, KeywordEnd, KeywordPrint, KeywordPrintln,
        KeywordNil, KeywordIf, KeywordThen, KeywordElse, KeywordTrue, KeywordFalse,
        KeywordFor, KeywordIn, KeywordWhile, KeywordBreak, KeywordContinue,
        EndOfFile, Unknown
    };

    struct Token {
        TokenType type;
        std::string text;
        int line = 1;
        int column = 1;
    };
} //Interpreter