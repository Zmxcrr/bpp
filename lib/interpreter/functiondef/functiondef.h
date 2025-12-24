#pragma once

#include "environment.h"
#include <vector>
#include <memory>

namespace Interpreter {
    struct Environment;
    struct Stmt;

    struct FunctionDef {
        std::vector<std::string> params;
        std::vector<std::unique_ptr<Stmt> > bodyStmts;
        Environment* closure{nullptr};
        std::string name = "<anonymous>";
    };
} //Interpreter

