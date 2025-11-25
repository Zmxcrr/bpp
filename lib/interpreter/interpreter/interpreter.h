#pragma once
#include <string_view>
#include "interpreter/token/token.h"
#include "interpreter/lexer/lexer.h"
#include "interpreter/parser/parser.h"
#include "interpreter/environment/environment.h"
#include "interpreter/exception/exception.h"
#include <sstream>
#include <vector>

namespace Interpreter {
    extern std::istream* CurrentIn;
    extern std::ostream* CurrentOut;

    inline std::vector<std::string> callStack;
    inline size_t MAX_CALL_DEPTH = 10000000000;

    bool interpret(std::istream &in, std::ostream &out);
} //Interpreter