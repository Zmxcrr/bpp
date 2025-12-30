#pragma once
#include "../vm/bytecode.h"
#include "../interpreter/statement/statement.h"
#include "../interpreter/value/value.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace BaldVM {

class Compiler {
public:
    struct CompiledProgram {
        std::vector<Instruction> bytecode;
        std::vector<Interpreter::Value> constants;
        std::vector<std::string> variable_names;
    };
    
    Compiler() = default;
    
    void setOptimizationLevel(int level) { optimization_level_ = level; }
    
    CompiledProgram compile(const std::vector<std::unique_ptr<Interpreter::Stmt>>& statements);
    
private:
    void compileStatement(const Interpreter::Stmt& stmt);
    void compileExpression(const Interpreter::Expr& expr);
    void emit(OpCode opcode, int32_t operand = 0);
    int32_t addConstant(const Interpreter::Value& val);
    int32_t addVariableName(const std::string& name);
    
    std::vector<Instruction> bytecode_;
    std::vector<Interpreter::Value> constants_;
    std::vector<std::string> variable_names_;
    std::unordered_map<std::string, int32_t> variable_indices_;
    
    int optimization_level_ = 0;  // <-- ДОБАВЬ ЭТУ СТРОКУ
};

} // namespace BaldVM

