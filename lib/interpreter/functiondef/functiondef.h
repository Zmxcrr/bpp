#pragma once

#include "../environment/environment.h"
#include "vm/bytecode.h"
#include <vector>
#include <memory>

namespace Interpreter {
    struct Environment;
    struct Stmt;

    struct FunctionDef {
        std::vector<std::string> params;
        std::vector<std::unique_ptr<Stmt> > bodyStmts;
	std::vector<BaldVM::Instruction> bytecode;
        Environment* closure{nullptr};
        std::string name = "<anonymous>";
    };
} //Interpreter

