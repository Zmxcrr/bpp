#ifndef SIMPLE_JIT_H
#define SIMPLE_JIT_H

#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <cstdint>
#include <string>

namespace Interpreter {

struct Value;
struct Environment;
class Expr;
class Stmt;

namespace SimpleJIT {

enum class OpCode : uint8_t {
    LOAD_NUMBER,
    LOAD_STRING,
    LOAD_BOOL, 
    LOAD_NIL, 

    LOAD_VAR,
    STORE_VAR,

    ADD, SUB, MUL, DIV, MOD, POW,

    EQ, NE, LT, LE, GT, GE,

    AND, OR, NOT,

    NEG, POS,

    CALL, BUILTIN_CALL,

    POP, DUP,

    JUMP, JUMP_IF_FALSE, JUMP_IF_TRUE,

    RETURN,

    MAKE_ARRAY, INDEX,

    HALT
};

struct Instruction {
    OpCode opcode;
    uint32_t operand = 0;

    Instruction(OpCode op, uint32_t arg = 0) : opcode(op), operand(arg) {}
};

struct CompiledFunction {
    std::string name;
    std::vector<Instruction> bytecode;
    std::vector<Value> constants;
    std::vector<std::string> var_names;
    int call_count = 0;
    bool is_compiled = false;

    CompiledFunction() = default;
    explicit CompiledFunction(const std::string& n) : name(n) {}
};

class BytecodeVM {
private:
    std::vector<Value> stack;
    std::vector<size_t> call_stack;
    size_t instruction_pointer = 0;
    size_t instructions_executed = 0;
    size_t stack_peak = 0;

public:
    BytecodeVM();

    Value execute(const CompiledFunction& func, Environment& env);

    void push(const Value& value);
    Value pop();
    Value& top();
    void clear();

    struct Stats {
        size_t instructions_executed;
        size_t stack_peak;
        size_t current_stack_size;
    };
    Stats getStats() const;
    void resetStats();

private:
    void executeInstruction(const Instruction& instr, const CompiledFunction& func, Environment& env);
};

class ExpressionCompiler {
private:
    std::vector<Instruction> bytecode;
    std::vector<Value> constants;
    std::vector<std::string> var_names;
    std::map<std::string, uint32_t> var_name_indices;

public:
    ExpressionCompiler();

    CompiledFunction compile(const Expr& expr, const std::string& name = std::string());

    void compileExpression(const Expr& expr);

private:
    uint32_t addConstant(const Value& value);
    uint32_t addVariableName(const std::string& name);
    void emit(OpCode opcode, uint32_t operand = 0);
    void optimizeBytecode();
};

class AdaptiveJIT {
private:
    std::unique_ptr<BytecodeVM> vm;
    std::unique_ptr<ExpressionCompiler> compiler;
    std::map<std::string, CompiledFunction> compiled_cache;

    int compilation_threshold = 3;
    bool optimization_enabled = true;

    size_t expressions_compiled = 0;
    size_t expressions_interpreted = 0;
    size_t cache_hits = 0;
    size_t cache_misses = 0;

public:
    AdaptiveJIT();
    ~AdaptiveJIT();

    bool initialize();
    void shutdown();

    void setCompilationThreshold(int threshold);
    void setOptimizationEnabled(bool enabled);

    Value evaluateExpression(const Expr& expr, Environment& env);

    bool shouldCompile(const std::string& expr_key, int call_count) const;
    std::string generateExpressionKey(const Expr& expr) const;
    void clearCache();

    struct Stats {
        size_t expressions_compiled;
        size_t expressions_interpreted; 
        size_t cache_hits;
        size_t cache_misses;
        size_t active_functions;
        BytecodeVM::Stats vm_stats;
    };
    Stats getStats() const;

    void dumpBytecode(const std::string& name) const;

private:
    CompiledFunction* getCachedFunction(const std::string& key);
    void cacheFunction(const std::string& key, CompiledFunction&& func);
    Value interpretExpression(const Expr& expr, Environment& env);
};

extern std::unique_ptr<AdaptiveJIT> GlobalSimpleJIT;

bool initializeSimpleJIT();
void shutdownSimpleJIT();
void setJITThreshold(int threshold);
void setJITOptimization(bool enabled);

Value evaluateWithSimpleJIT(const Expr& expr, Environment& env);

std::string opcodeToString(OpCode opcode);
void printBytecode(const CompiledFunction& func);
void printInstruction(const Instruction& instr, size_t index);

} // namespace SimpleJIT
} // namespace Interpreter

#endif // SIMPLE_JIT_H