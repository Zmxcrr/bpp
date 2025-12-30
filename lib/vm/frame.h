#pragma once
#include "../interpreter/value/value.h"
#include "../interpreter/functiondef/functiondef.h"
#include <unordered_map>
#include <memory>
#include <string>

namespace BaldVM {

struct Frame {
    std::shared_ptr<Interpreter::FunctionDef> function;
    std::unordered_map<std::string, Interpreter::Value> locals;
    size_t return_ip = 0;  // где продолжить после return
    const std::vector<Instruction>* code = nullptr;  // на какой байткод смотрим
};

} // namespace BaldVM

