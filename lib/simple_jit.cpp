#include "simple_jit.h"
#include "interpreter/value/value.h"
#include "interpreter/environment/environment.h"
#include "interpreter/expression/expression.h"
#include "interpreter.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <typeinfo>
#include <iomanip>
#include <cmath>

namespace Interpreter {
namespace SimpleJIT {

std::unique_ptr<AdaptiveJIT> GlobalSimpleJIT = nullptr;

BytecodeVM::BytecodeVM() {
    stack.reserve(256);
}

Value BytecodeVM::execute(const CompiledFunction& func, Environment& env) {
    if (func.bytecode.empty()) {
        return Value();
    }

    instruction_pointer = 0;
    stack.clear();

    try {
        while (instruction_pointer < func.bytecode.size()) {
            const auto& instr = func.bytecode[instruction_pointer];
            executeInstruction(instr, func, env);

            instructions_executed++;
            stack_peak = std::max(stack_peak, stack.size());

            if (instr.opcode == OpCode::HALT || instr.opcode == OpCode::RETURN) {
                break;
            }

            instruction_pointer++;
        }

        return stack.empty() ? Value() : pop();

    } catch (const std::exception& e) {
        std::cerr << "VM Error: " << e.what() << std::endl;
        stack.clear();
        return Value();
    }
}

void BytecodeVM::executeInstruction(const Instruction& instr, const CompiledFunction& func, Environment& env) {
    switch (instr.opcode) {
        case OpCode::LOAD_NUMBER:
        case OpCode::LOAD_STRING:
        case OpCode::LOAD_BOOL:
        case OpCode::LOAD_NIL: {
            if (instr.operand >= func.constants.size()) {
                throw std::runtime_error("Invalid constant index");
            }
            push(func.constants[instr.operand]);
            break;
        }

        case OpCode::LOAD_VAR: {
            if (instr.operand >= func.var_names.size()) {
                throw std::runtime_error("Invalid variable name index");
            }
            const std::string& var_name = func.var_names[instr.operand];
            try {
                Value var_value = env.get(var_name);
                push(var_value);
            } catch (const std::runtime_error&) {
                throw std::runtime_error("Undefined variable: " + var_name);
            }
            break;
        }

        case OpCode::STORE_VAR: {
            if (instr.operand >= func.var_names.size()) {
                throw std::runtime_error("Invalid variable name index");
            }
            const std::string& var_name = func.var_names[instr.operand];
            Value value = pop();
            env.set(var_name, value);
            push(value);
            break;
        }

        case OpCode::ADD: {
            Value b = pop(), a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                push(Value(a.numberValue + b.numberValue));
            } else {
                push(Value());
            }
            break;
        }

        case OpCode::SUB: {
            Value b = pop(), a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                push(Value(a.numberValue - b.numberValue));
            } else {
                push(Value());
            }
            break;
        }

        case OpCode::MUL: {
            Value b = pop(), a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                push(Value(a.numberValue * b.numberValue));
            } else {
                push(Value());
            }
            break;
        }

        case OpCode::DIV: {
            Value b = pop(), a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER && b.numberValue != 0) {
                push(Value(a.numberValue / b.numberValue));
            } else {
                push(Value());
            }
            break;
        }

        case OpCode::LT: {
            Value b = pop(), a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                push(Value(a.numberValue < b.numberValue));
            } else {
                push(Value(false));
            }
            break;
        }

        case OpCode::GT: {
            Value b = pop(), a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                push(Value(a.numberValue > b.numberValue));
            } else {
                push(Value(false));
            }
            break;
        }

        case OpCode::EQ: {
            Value b = pop(), a = pop();
            bool equal = false;
            if (a.type == b.type) {
                switch (a.type) {
                    case ValueType::NUMBER: equal = (a.numberValue == b.numberValue); break;
                    case ValueType::BOOL: equal = (a.boolValue == b.boolValue); break;
                    case ValueType::STRING: equal = (*a.stringValue == *b.stringValue); break;
                    case ValueType::NIL: equal = true; break;
                    default: equal = false; break;
                }
            }
            push(Value(equal));
            break;
        }

        case OpCode::NEG: {
            Value a = pop();
            if (a.type == ValueType::NUMBER) {
                push(Value(-a.numberValue));
            } else {
                push(Value());
            }
            break;
        }

        case OpCode::NOT: {
            Value a = pop();
            push(Value(!a.toBool()));
            break;
        }

        case OpCode::POP: {
            pop();
            break;
        }

        case OpCode::DUP: {
            push(top());
            break;
        }

        case OpCode::RETURN:
        case OpCode::HALT:
            break;

    case OpCode::MOD: {
        Value b = pop(), a = pop();
        if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER && b.numberValue != 0) {
            push(Value(std::fmod(a.numberValue, b.numberValue)));
        } else {
            push(Value());
        }
        break;
    }

    case OpCode::POW: {
        Value b = pop(), a = pop();
        if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
            push(Value(std::pow(a.numberValue, b.numberValue)));
        } else {
            push(Value());
        }
        break;
    }

    case OpCode::NE: {
        Value b = pop(), a = pop();
        bool equal = false;
        if (a.type == b.type) {
            switch (a.type) {
                case ValueType::NUMBER: equal = (a.numberValue == b.numberValue); break;
                case ValueType::BOOL: equal = (a.boolValue == b.boolValue); break;
                case ValueType::STRING: equal = (*a.stringValue == *b.stringValue); break;
                case ValueType::NIL: equal = true; break;
                default: equal = false; break;
            }
        }
        push(Value(!equal));
        break;
    }

    case OpCode::LE: {
        Value b = pop(), a = pop();
        if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
            push(Value(a.numberValue <= b.numberValue));
        } else {
            push(Value(false));
        }
        break;
    }

    case OpCode::GE: {
        Value b = pop(), a = pop();
        if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
            push(Value(a.numberValue >= b.numberValue));
        } else {
            push(Value(false));
        }
        break;
    }

    case OpCode::AND: {
        Value b = pop(), a = pop();
        push(Value(a.toBool() && b.toBool()));
        break;
    }

    case OpCode::OR: {
        Value b = pop(), a = pop();
        push(Value(a.toBool() || b.toBool()));
        break;
    }

    case OpCode::POS: {
        Value a = pop();
        if (a.type == ValueType::NUMBER) {
            push(a);
        } else {
            push(Value());
        }
        break;
    }

    case OpCode::JUMP: {
        if (instr.operand >= func.bytecode.size()) {
            throw std::runtime_error("Invalid jump target");
        }
        instruction_pointer = instr.operand - 1; // -1 потому что в execute() будет ++
        break;
    }

    case OpCode::JUMP_IF_FALSE: {
        Value condition = pop();
        if (!condition.toBool()) {
            if (instr.operand >= func.bytecode.size()) {
                throw std::runtime_error("Invalid jump target");
            }
            instruction_pointer = instr.operand - 1;
        }
        break;
    }

    case OpCode::JUMP_IF_TRUE: {
        Value condition = pop();
        if (condition.toBool()) {
            if (instr.operand >= func.bytecode.size()) {
                throw std::runtime_error("Invalid jump target");
            }
            instruction_pointer = instr.operand - 1;
        }
        break;
    }

    case OpCode::MAKE_ARRAY: {
        size_t array_size = instr.operand;
        std::vector<Value> elements;
        elements.reserve(array_size);
        
        for (size_t i = 0; i < array_size; i++) {
            elements.insert(elements.begin(), pop());
        }
        
        push(Value(std::move(elements)));
        break;
    }

    case OpCode::INDEX: {
        Value index_val = pop();
        Value array_val = pop();
        
        if (array_val.type == ValueType::ARRAY && index_val.type == ValueType::NUMBER) {
            size_t idx = static_cast<size_t>(index_val.numberValue);
            if (idx < array_val.arrayValue->size()) {
                push((*array_val.arrayValue)[idx]);
            } else {
                push(Value());
            }
        } else if (array_val.type == ValueType::STRING && index_val.type == ValueType::NUMBER) {
            size_t idx = static_cast<size_t>(index_val.numberValue);
            if (idx < array_val.stringValue->size()) {
                push(Value(std::string(1, (*array_val.stringValue)[idx])));
            } else {
                push(Value());
            }
        } else {
            push(Value());
        }
        break;
    }

    case OpCode::CALL: {
        std::vector<Value> args;
        args.reserve(instr.operand);
        for (size_t i = 0; i < instr.operand; i++) {
            args.insert(args.begin(), pop());
        }
        
        Value func_val = pop();
        if (func_val.type == ValueType::FUNCTION) {
            push(Value());
        } else {
            throw std::runtime_error("Not a function");
        }
        break;
    }

    case OpCode::BUILTIN_CALL: {
        if (instr.operand >= func.var_names.size()) {
            throw std::runtime_error("Invalid builtin name index");
        }
        
        const std::string& builtin_name = func.var_names[instr.operand];
        
        if (builtin_name == "print") {
            Value arg = pop();
            push(Value());
        } else if (builtin_name == "len") {
            Value arg = pop();
            if (arg.type == ValueType::STRING) {
                push(Value(static_cast<double>(arg.stringValue->size())));
            } else if (arg.type == ValueType::ARRAY) {
                push(Value(static_cast<double>(arg.arrayValue->size())));
            } else {
                push(Value());
            }
        } else {
            throw std::runtime_error("Unknown builtin: " + builtin_name);
        }
        break;
    }

        default:
            throw std::runtime_error("Unsupported opcode: " + std::to_string(static_cast<int>(instr.opcode)));
    }
}

void BytecodeVM::push(const Value& value) {
    stack.push_back(value);
}

Value BytecodeVM::pop() {
    if (stack.empty()) {
        throw std::runtime_error("Stack underflow");
    }
    Value value = std::move(stack.back());
    stack.pop_back();
    return value;
}

Value& BytecodeVM::top() {
    if (stack.empty()) {
        throw std::runtime_error("Empty stack");
    }
    return stack.back();
}

void BytecodeVM::clear() {
    stack.clear();
    instruction_pointer = 0;
}

BytecodeVM::Stats BytecodeVM::getStats() const {
    return {
        instructions_executed,
        stack_peak,
        stack.size()
    };
}

void BytecodeVM::resetStats() {
    instructions_executed = 0;
    stack_peak = 0;
}

ExpressionCompiler::ExpressionCompiler() {
    bytecode.reserve(64);
    constants.reserve(16);
    var_names.reserve(8);
}

CompiledFunction ExpressionCompiler::compile(const Expr& expr, const std::string& name) {
    bytecode.clear();
    constants.clear();
    var_names.clear();
    var_name_indices.clear();

    compileExpression(expr);
    
    optimizeBytecode();
    
    emit(OpCode::RETURN);

    CompiledFunction func(name.empty() ? "anonymous" : name);
    func.bytecode = std::move(bytecode);
    func.constants = std::move(constants);
    func.var_names = std::move(var_names);
    func.is_compiled = true;

    return func;
}

void ExpressionCompiler::compileExpression(const Expr& expr) {
    if (auto* num_expr = dynamic_cast<const NumberExpr*>(&expr)) {
        emit(OpCode::LOAD_NUMBER, addConstant(Value(num_expr->value)));
    }
    else if (auto* str_expr = dynamic_cast<const StringExpr*>(&expr)) {
        emit(OpCode::LOAD_STRING, addConstant(Value(str_expr->value)));
    }
    else if (auto* bool_expr = dynamic_cast<const BoolExpr*>(&expr)) {
        emit(OpCode::LOAD_BOOL, addConstant(Value(bool_expr->value)));
    }
    else if (auto* nil_expr [[maybe_unused]] = dynamic_cast<const NilExpr*>(&expr)) {
        emit(OpCode::LOAD_NIL, addConstant(Value()));
    }
    else if (auto* var_expr = dynamic_cast<const VariableExpr*>(&expr)) {
        emit(OpCode::LOAD_VAR, addVariableName(var_expr->name));
    }
    else if (auto* bin_expr = dynamic_cast<const BinaryExpr*>(&expr)) {
        if (bin_expr->op == "and") {
            compileExpression(*bin_expr->left);
            emit(OpCode::DUP);
            size_t jump_end = bytecode.size();
            emit(OpCode::JUMP_IF_FALSE, 0);
            emit(OpCode::POP);
            compileExpression(*bin_expr->right);
            bytecode[jump_end].operand = static_cast<uint32_t>(bytecode.size());
            return;
        }
        else if (bin_expr->op == "or") {
            compileExpression(*bin_expr->left);
            emit(OpCode::DUP);
            size_t jump_end = bytecode.size();
            emit(OpCode::JUMP_IF_TRUE, 0);
            emit(OpCode::POP);
            compileExpression(*bin_expr->right);
            bytecode[jump_end].operand = static_cast<uint32_t>(bytecode.size());
            return;
        }
        
        compileExpression(*bin_expr->left);
        compileExpression(*bin_expr->right);

        if (bin_expr->op == "+") emit(OpCode::ADD);
        else if (bin_expr->op == "-") emit(OpCode::SUB);
        else if (bin_expr->op == "*") emit(OpCode::MUL);
        else if (bin_expr->op == "/") emit(OpCode::DIV);
        else if (bin_expr->op == "%") emit(OpCode::MOD);
        else if (bin_expr->op == "**" || bin_expr->op == "^") emit(OpCode::POW);
        else if (bin_expr->op == "<") emit(OpCode::LT);
        else if (bin_expr->op == "<=") emit(OpCode::LE);
        else if (bin_expr->op == ">") emit(OpCode::GT);
        else if (bin_expr->op == ">=") emit(OpCode::GE);
        else if (bin_expr->op == "==") emit(OpCode::EQ);
        else if (bin_expr->op == "!=") emit(OpCode::NE);
        else {
            emit(OpCode::POP);
            emit(OpCode::POP);
            emit(OpCode::LOAD_NIL, addConstant(Value()));
        }
    }
    else if (auto* unary_expr = dynamic_cast<const UnaryExpr*>(&expr)) {
        compileExpression(*unary_expr->operand);

        if (unary_expr->op == "-") emit(OpCode::NEG);
        else if (unary_expr->op == "+") emit(OpCode::POS);
        else if (unary_expr->op == "not" || unary_expr->op == "!") emit(OpCode::NOT);
        else {
            emit(OpCode::POP);
            emit(OpCode::LOAD_NIL, addConstant(Value()));
        }
    }
    else if (auto* array_expr = dynamic_cast<const ArrayExpr*>(&expr)) {
        for (const auto& elem : array_expr->elements) {
            compileExpression(*elem);
        }
        emit(OpCode::MAKE_ARRAY, static_cast<uint32_t>(array_expr->elements.size()));
    }
    else if (auto* index_expr = dynamic_cast<const IndexExpr*>(&expr)) {
        compileExpression(*index_expr->base);
        compileExpression(*index_expr->index);
        emit(OpCode::INDEX);
    }
    else if (auto* call_expr = dynamic_cast<const CallExpr*>(&expr)) {
        for (const auto& arg : call_expr->args) {
            compileExpression(*arg);
        }
        
        if (auto* var = dynamic_cast<const VariableExpr*>(call_expr->callee.get())) {
            emit(OpCode::LOAD_VAR, addVariableName(var->name));
            emit(OpCode::CALL, static_cast<uint32_t>(call_expr->args.size()));
        } else {
            compileExpression(*call_expr->callee);
            emit(OpCode::CALL, static_cast<uint32_t>(call_expr->args.size()));
        }
    }
    else {
        emit(OpCode::LOAD_NIL, addConstant(Value()));
    }
}

uint32_t ExpressionCompiler::addConstant(const Value& value) {
    constants.push_back(value);
    return static_cast<uint32_t>(constants.size() - 1);
}

uint32_t ExpressionCompiler::addVariableName(const std::string& name) {
    if (auto it = var_name_indices.find(name); it != var_name_indices.end()) {
        return it->second;
    }

    uint32_t index = static_cast<uint32_t>(var_names.size());
    var_names.push_back(name);
    var_name_indices[name] = index;
    return index;
}

void ExpressionCompiler::emit(OpCode opcode, uint32_t operand) {
    bytecode.emplace_back(opcode, operand);
}

void ExpressionCompiler::optimizeBytecode() {
    if (bytecode.size() < 2) return;
    
    std::vector<Instruction> optimized;
    optimized.reserve(bytecode.size());
    
    for (size_t i = 0; i < bytecode.size(); ) {
        bool optimized_pair = false;
        
        if (i + 1 < bytecode.size()) {
            const auto& instr1 = bytecode[i];
            const auto& instr2 = bytecode[i + 1];
            
            if ((instr1.opcode == OpCode::LOAD_NUMBER || 
                 instr1.opcode == OpCode::LOAD_STRING ||
                 instr1.opcode == OpCode::LOAD_BOOL ||
                 instr1.opcode == OpCode::LOAD_NIL) && 
                instr2.opcode == OpCode::POP) {
                i += 2;
                optimized_pair = true;
            }
            else if (instr1.opcode == OpCode::DUP && instr2.opcode == OpCode::POP) {
                i += 2;
                optimized_pair = true;
            }
            else if (instr1.opcode == OpCode::NOT && instr2.opcode == OpCode::NOT) {
                i += 2;
                optimized_pair = true;
            }
            else if (instr1.opcode == OpCode::NEG && instr2.opcode == OpCode::NEG) {
                i += 2;
                optimized_pair = true;
            }
        }
        
        if (!optimized_pair && i + 2 < bytecode.size()) {
            const auto& instr1 = bytecode[i];
            const auto& instr2 = bytecode[i + 1];
            const auto& instr3 = bytecode[i + 2];
            
            if (instr1.opcode == OpCode::LOAD_NUMBER && 
                instr2.opcode == OpCode::LOAD_NUMBER &&
                instr1.operand < constants.size() &&
                instr2.operand < constants.size() &&
                constants[instr1.operand].type == ValueType::NUMBER &&
                constants[instr2.operand].type == ValueType::NUMBER) {
                
                double a = constants[instr1.operand].numberValue;
                double b = constants[instr2.operand].numberValue;
                double result = 0;
                bool can_fold = true;
                
                switch (instr3.opcode) {
                    case OpCode::ADD: result = a + b; break;
                    case OpCode::SUB: result = a - b; break;
                    case OpCode::MUL: result = a * b; break;
                    case OpCode::DIV:
                        if (b != 0) result = a / b;
                        else can_fold = false;
                        break;
                    default: can_fold = false; break;
                }
                
                if (can_fold) {
                    optimized.emplace_back(OpCode::LOAD_NUMBER, addConstant(Value(result)));
                    i += 3;
                    optimized_pair = true;
                }
            }
        }
        
        if (!optimized_pair) {
            optimized.push_back(bytecode[i]);
            i++;
        }
    }
    
    bytecode = std::move(optimized);
}

AdaptiveJIT::AdaptiveJIT() {
    vm = std::make_unique<BytecodeVM>();
    compiler = std::make_unique<ExpressionCompiler>();
}

AdaptiveJIT::~AdaptiveJIT() {
    shutdown();
}

bool AdaptiveJIT::initialize() {
    vm->resetStats();
    return true;
}

void AdaptiveJIT::shutdown() {
    clearCache();
    vm->resetStats();
}

Value AdaptiveJIT::evaluateExpression(const Expr& expr, Environment& env) {
    std::string expr_key = generateExpressionKey(expr);

    CompiledFunction* cached = getCachedFunction(expr_key);
    if (cached) {
        cached->call_count++;
        cache_hits++;

        if (cached->is_compiled) {
            return vm->execute(*cached, env);
        } else {
            if (shouldCompile(expr_key, cached->call_count)) {
                *cached = compiler->compile(expr, expr_key);
                expressions_compiled++;
                return vm->execute(*cached, env);
            } else {
                expressions_interpreted++;
                return interpretExpression(expr, env);
            }
        }
    } else {
        cache_misses++;
        CompiledFunction func(expr_key);
        func.is_compiled = false;
        func.call_count = 1;
        cacheFunction(expr_key, std::move(func));

        expressions_interpreted++;
        return interpretExpression(expr, env);
    }
}

bool AdaptiveJIT::shouldCompile([[maybe_unused]] const std::string& expr_key, int call_count) const {
    return call_count >= compilation_threshold;
}

std::string AdaptiveJIT::generateExpressionKey(const Expr& expr) const {
    // Generate a key based on expression structure
    std::ostringstream key;

    if (auto* num_expr = dynamic_cast<const NumberExpr*>(&expr)) {
        key << "num:" << num_expr->value;
    } else if (auto* var_expr = dynamic_cast<const VariableExpr*>(&expr)) {
        key << "var:" << var_expr->name;
    } else if (auto* bin_expr = dynamic_cast<const BinaryExpr*>(&expr)) {
        key << "bin:" << bin_expr->op << ":"
            << generateExpressionKey(*bin_expr->left) << ":"
            << generateExpressionKey(*bin_expr->right);
    } else {
        key << "expr:" << typeid(expr).name();
    }

    return key.str();
}

CompiledFunction* AdaptiveJIT::getCachedFunction(const std::string& key) {
    if (auto it = compiled_cache.find(key); it != compiled_cache.end()) {
        return &it->second;
    }
    return nullptr;
}

void AdaptiveJIT::cacheFunction(const std::string& key, CompiledFunction&& func) {
    compiled_cache[key] = std::move(func);
}

Value AdaptiveJIT::interpretExpression(const Expr& expr, Environment& env) {
    // Fallback to regular interpretation
    std::ostringstream dummy_out;
    return const_cast<Expr&>(expr).eval(&env, dummy_out);
}

void AdaptiveJIT::setCompilationThreshold(int threshold) {
    compilation_threshold = std::max(1, threshold);
}

void AdaptiveJIT::setOptimizationEnabled(bool enabled) {
    optimization_enabled = enabled;
}

void AdaptiveJIT::clearCache() {
    compiled_cache.clear();
}

AdaptiveJIT::Stats AdaptiveJIT::getStats() const {
    return {
        expressions_compiled,
        expressions_interpreted,
        cache_hits,
        cache_misses,
        compiled_cache.size(),
        vm->getStats()
    };
}

void AdaptiveJIT::dumpBytecode(const std::string& name) const {
    if (auto it = compiled_cache.find(name); it != compiled_cache.end() && it->second.is_compiled) {
        std::cout << "Bytecode for '" << name << "':" << std::endl;
        printBytecode(it->second);
    } else {
        std::cout << "No compiled bytecode found for '" << name << "'" << std::endl;
    }
}

bool initializeSimpleJIT() {
    if (!GlobalSimpleJIT) {
        GlobalSimpleJIT = std::make_unique<AdaptiveJIT>();
        return GlobalSimpleJIT->initialize();
    }
    return true;
}

void shutdownSimpleJIT() {
    if (GlobalSimpleJIT) {
        GlobalSimpleJIT->shutdown();
        GlobalSimpleJIT.reset();
    }
}

void setJITThreshold(int threshold) {
    if (GlobalSimpleJIT) {
        GlobalSimpleJIT->setCompilationThreshold(threshold);
    }
}

void setJITOptimization(bool enabled) {
    if (GlobalSimpleJIT) {
        GlobalSimpleJIT->setOptimizationEnabled(enabled);
    }
}

Value evaluateWithSimpleJIT(const Expr& expr, Environment& env) {
    if (GlobalSimpleJIT) {
        return GlobalSimpleJIT->evaluateExpression(expr, env);
    }
    std::ostringstream dummy_out;
    return const_cast<Expr&>(expr).eval(&env, dummy_out);
}

std::string opcodeToString(OpCode opcode) {
    using enum OpCode;

    switch (opcode) {
        case LOAD_NUMBER: return "LOAD_NUMBER";
        case LOAD_STRING: return "LOAD_STRING";
        case LOAD_BOOL: return "LOAD_BOOL";
        case LOAD_NIL: return "LOAD_NIL";
        case LOAD_VAR: return "LOAD_VAR";
        case STORE_VAR: return "STORE_VAR";
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case MOD: return "MOD";
        case POW: return "POW";
        case EQ: return "EQ";
        case NE: return "NE";
        case LT: return "LT";
        case LE: return "LE";
        case GT: return "GT";
        case GE: return "GE";
        case AND: return "AND";
        case OR: return "OR";
        case NOT: return "NOT";
        case NEG: return "NEG";
        case POS: return "POS";
        case CALL: return "CALL";
        case BUILTIN_CALL: return "BUILTIN_CALL";
        case POP: return "POP";
        case DUP: return "DUP";
        case JUMP: return "JUMP";
        case JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case JUMP_IF_TRUE: return "JUMP_IF_TRUE";
        case RETURN: return "RETURN";
        case MAKE_ARRAY: return "MAKE_ARRAY";
        case INDEX: return "INDEX";
        case HALT: return "HALT";
        default: return "UNKNOWN";
    }
}

void printInstruction(const Instruction& instr, size_t index) {
    std::cout << std::setfill('0') << std::setw(4) << index << ": "
              << opcodeToString(instr.opcode);
    if (instr.operand != 0) {
        std::cout << " " << instr.operand;
    }
    std::cout << std::endl;
}

void printBytecode(const CompiledFunction& func) {
    std::cout << "=== " << func.name << " ===" << std::endl;

    if (!func.constants.empty()) {
        std::cout << "Constants:" << std::endl;
        for (size_t i = 0; i < func.constants.size(); i++) {
            const auto& val = func.constants[i];
            std::cout << "  [" << i << "] ";

            switch (val.type) {
                case ValueType::NUMBER:
                    std::cout << val.numberValue;
                    break;
                case ValueType::STRING:
                    std::cout << '"' << *val.stringValue << '"';
                    break;
                case ValueType::BOOL:
                    std::cout << (val.boolValue ? "true" : "false");
                    break;
                default:
                    std::cout << "nil";
                    break;
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    if (!func.var_names.empty()) {
        std::cout << "Variables:" << std::endl;
        for (size_t i = 0; i < func.var_names.size(); i++) {
            std::cout << "  [" << i << "] " << func.var_names[i] << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "Bytecode:" << std::endl;
    for (size_t i = 0; i < func.bytecode.size(); i++) {
        printInstruction(func.bytecode[i], i);
    }
    std::cout << std::endl;
}

} // namespace SimpleJIT
} // namespace Interpreter

