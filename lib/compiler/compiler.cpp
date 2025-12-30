#include "compiler.h"
#include "../optimizer/const_folding.h"
#include "../optimizer/dead_code.h"
#include "../interpreter/functiondef/functiondef.h"  
#include "../interpreter/expression/expression.h" 
#include <stdexcept>
#include <unordered_set>
#include <cmath>
#include <iostream>

namespace BaldVM {

using namespace Interpreter;

Compiler::CompiledProgram Compiler::compile(
    const std::vector<std::unique_ptr<Stmt>>& statements) {
    
    bytecode_.clear();
    constants_.clear();
    variable_names_.clear();
    variable_indices_.clear();
    
    // Compile all statements
    for (const auto& stmt : statements) {
        if (stmt) {
            compileStatement(*stmt);
        }
    }
    
    emit(OpCode::HALT);
    
    // Применяем оптимизации если включены
    if (optimization_level_ > 0) {
        Optimizer::ConstantFoldingOptimizer const_fold;
        const_fold.optimize(bytecode_, constants_);
        
        Optimizer::DeadCodeOptimizer dead_code;
        dead_code.optimize(bytecode_, constants_);
    }
    
    CompiledProgram program;
    program.bytecode = std::move(bytecode_);
    program.constants = std::move(constants_);
    program.variable_names = std::move(variable_names_);
    
    return program;
}

void Compiler::compileStatement(const Stmt& stmt) {
    // ExpressionStmt
    if (auto* expr_stmt = dynamic_cast<const ExpressionStmt*>(&stmt)) {
        compileExpression(*expr_stmt->expr);
        emit(OpCode::POP);  // Discard result
        return;
    }
    
    // AssignStmt: name = expr
    if (auto* assign_stmt = dynamic_cast<const AssignStmt*>(&stmt)) {
        compileExpression(*assign_stmt->expr);
        emit(OpCode::STORE_VAR, addVariableName(assign_stmt->name));
        return;
    }
    
    // PrintlnStmt
    if (auto* print_stmt = dynamic_cast<const PrintlnStmt*>(&stmt)) {
        compileExpression(*print_stmt->expr);
        emit(OpCode::PRINT);
        emit(OpCode::POP);
        return;
    }
    
    // IfStmt
    if (auto* if_stmt = dynamic_cast<const IfStmt*>(&stmt)) {
        // Compile condition
        compileExpression(*if_stmt->condition);
        
        // JUMP_IF_FALSE to else/else-if branch or end
        size_t jump_to_else = bytecode_.size();
        emit(OpCode::JUMP_IF_FALSE, 0);  // Placeholder
        
        // Compile then branch
        for (const auto& then_stmt : if_stmt->thenBranch) {
            compileStatement(*then_stmt);
        }
        
        // Handle else-if branches
        std::vector<size_t> jump_to_end_positions;
        
        if (!if_stmt->elseIfBranches.empty() || !if_stmt->elseBranch.empty()) {
            // Jump over other branches after then
            jump_to_end_positions.push_back(bytecode_.size());
            emit(OpCode::JUMP, 0);  // Placeholder
        }
        
        // Patch jump_to_else
        bytecode_[jump_to_else].operand = bytecode_.size();
        
        // Compile else-if branches
        for (const auto& [else_if_cond, else_if_body] : if_stmt->elseIfBranches) {
            compileExpression(*else_if_cond);
            
            size_t jump_to_next = bytecode_.size();
            emit(OpCode::JUMP_IF_FALSE, 0);
            
            for (const auto& stmt : else_if_body) {
                compileStatement(*stmt);
            }
            
            jump_to_end_positions.push_back(bytecode_.size());
            emit(OpCode::JUMP, 0);
            
            bytecode_[jump_to_next].operand = bytecode_.size();
        }
        
        // Compile else branch
        if (!if_stmt->elseBranch.empty()) {
            for (const auto& else_stmt : if_stmt->elseBranch) {
                compileStatement(*else_stmt);
            }
        }
        
        // Patch all jumps to end
        for (size_t pos : jump_to_end_positions) {
            bytecode_[pos].operand = bytecode_.size();
        }
        
        return;
    }
    
    // WhileStmt
    if (auto* while_stmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        size_t loop_start = bytecode_.size();
        
        // Compile condition
        compileExpression(*while_stmt->condition);
        
        // JUMP_IF_FALSE to end
        size_t jump_to_end = bytecode_.size();
        emit(OpCode::JUMP_IF_FALSE, 0);  // Placeholder
        
        // Compile body
        for (const auto& body_stmt : while_stmt->body) {
            compileStatement(*body_stmt);
        }
        
        // Jump back to loop start
        emit(OpCode::JUMP, loop_start);
        
        // Patch jump_to_end
        bytecode_[jump_to_end].operand = bytecode_.size();
        return;
    }
    
    // ForStmt: for var in iterable { ... }
    if (auto* for_stmt = dynamic_cast<const ForStmt*>(&stmt)) {
        // Evaluate iterable (must be array)
        compileExpression(*for_stmt->iterable);
        
        // Store in temporary variable __iter
        int32_t iter_var = addVariableName("__iter");
        emit(OpCode::STORE_VAR, iter_var);
        
        // Initialize index: __index = 0
        emit(OpCode::PUSH_NUMBER, addConstant(Value(0.0)));
        int32_t index_var = addVariableName("__index");
        emit(OpCode::STORE_VAR, index_var);
        
        size_t loop_start = bytecode_.size();
        
        // Load index
        emit(OpCode::LOAD_VAR, index_var);
        
        // Load array length
        emit(OpCode::LOAD_VAR, iter_var);
        emit(OpCode::ARRAY_LEN);
        
        // Compare: index < length
        emit(OpCode::LT);
        
        // JUMP_IF_FALSE to end
        size_t jump_to_end = bytecode_.size();
        emit(OpCode::JUMP_IF_FALSE, 0);
        
        // Load array[index] into loop variable
        emit(OpCode::LOAD_VAR, iter_var);
        emit(OpCode::LOAD_VAR, index_var);
        emit(OpCode::ARRAY_GET);
        emit(OpCode::STORE_VAR, addVariableName(for_stmt->var));  // <-- ИСПРАВЛЕНО: var вместо varName
        
        // Compile body
        for (const auto& body_stmt : for_stmt->body) {
            compileStatement(*body_stmt);
        }
        
        // Increment index: __index = __index + 1
        emit(OpCode::LOAD_VAR, index_var);
        emit(OpCode::PUSH_NUMBER, addConstant(Value(1.0)));
        emit(OpCode::ADD);
        emit(OpCode::STORE_VAR, index_var);
        
        // Jump back to loop start
        emit(OpCode::JUMP, loop_start);
        
        // Patch jump_to_end
        bytecode_[jump_to_end].operand = bytecode_.size();
        return;
    }

	// FunctionStmt: function name(params) ... end function
	if (auto* func_stmt = dynamic_cast<const FunctionStmt*>(&stmt)) {
	    // 1. Создать FunctionDef
	    auto fdef = std::make_shared<Interpreter::FunctionDef>();
	    fdef->params = func_stmt->params;
	    fdef->name = func_stmt->name;
	    
	    // 2. Временно переключить компиляцию
	    auto saved_bytecode = std::move(bytecode_);
	    bytecode_.clear();
	    
	    for (const auto& body_stmt : func_stmt->body) {
		compileStatement(*body_stmt);
	    }
	    // Гарантируем RETURN
	    emit(OpCode::PUSH_NIL, 0);
	    emit(OpCode::RETURN, 0);
	    
	    fdef->bytecode = std::move(bytecode_);
	    bytecode_ = std::move(saved_bytecode);
	    
	    // 3. Добавить как константу
	    constants_.push_back(Interpreter::Value(fdef));
	    int const_idx = constants_.size() - 1;
	    
	    // 4. PUSH и STORE в переменную
	    emit(OpCode::PUSH_NUMBER, const_idx);
	    emit(OpCode::STORE_VAR, addVariableName(func_stmt->name));
	    emit(OpCode::POP);
	    return;
	}
	    
	// ReturnStmt
	if (auto* ret_stmt = dynamic_cast<const ReturnStmt*>(&stmt)) {
	    if (ret_stmt->expr) {
		compileExpression(*ret_stmt->expr);
	    } else {
		emit(OpCode::PUSH_NIL, addConstant(Value()));
	    }
	    emit(OpCode::RETURN, 0);
	    return;
	}

	// AssignIndexStmt: arr[index] = value
	if (auto* assign_idx = dynamic_cast<const AssignIndexStmt*>(&stmt)) {
	    compileExpression(*assign_idx->base);     // массив
	    compileExpression(*assign_idx->index);    // индекс
	    compileExpression(*assign_idx->value);    // значение
	    emit(OpCode::ARRAY_SET, 0);
	    return;
	}
	std::cerr << "Unsupported statement: " << typeid(stmt).name() << std::endl;
    throw std::runtime_error("Unsupported statement type");
}

void Compiler::compileExpression(const Expr& expr) {
    // NumberExpr
    if (auto* num = dynamic_cast<const NumberExpr*>(&expr)) {
        emit(OpCode::PUSH_NUMBER, addConstant(Value(num->value)));
        return;
    }
    
    // StringExpr
    if (auto* str = dynamic_cast<const StringExpr*>(&expr)) {
        emit(OpCode::PUSH_STRING, addConstant(Value(str->value)));
        return;
    }
    
    // BoolExpr
    if (auto* b = dynamic_cast<const BoolExpr*>(&expr)) {
        emit(OpCode::PUSH_BOOL, addConstant(Value(b->value)));
        return;
    }
    
    // NilExpr
    if (dynamic_cast<const NilExpr*>(&expr)) {
        emit(OpCode::PUSH_NIL, addConstant(Value()));
        return;
    }
    
    // VariableExpr
    if (auto* var = dynamic_cast<const VariableExpr*>(&expr)) {
        emit(OpCode::LOAD_VAR, addVariableName(var->name));
        return;
    }
    
    // BinaryExpr
    if (auto* bin = dynamic_cast<const BinaryExpr*>(&expr)) {
        // Short-circuit evaluation for 'and' and 'or'
        if (bin->op == "and") {
            compileExpression(*bin->left);
            emit(OpCode::DUP);
            size_t jump_end = bytecode_.size();
            emit(OpCode::JUMP_IF_FALSE, 0);
            emit(OpCode::POP);
            compileExpression(*bin->right);
            bytecode_[jump_end].operand = bytecode_.size();
            return;
        }
        
        if (bin->op == "or") {
            compileExpression(*bin->left);
            emit(OpCode::DUP);
            size_t jump_end = bytecode_.size();
            emit(OpCode::JUMP_IF_TRUE, 0);
            emit(OpCode::POP);
            compileExpression(*bin->right);
            bytecode_[jump_end].operand = bytecode_.size();
            return;
        }
        
        // Regular binary operations
        compileExpression(*bin->left);
        compileExpression(*bin->right);
        
        if (bin->op == "+") emit(OpCode::ADD);
        else if (bin->op == "-") emit(OpCode::SUB);
        else if (bin->op == "*") emit(OpCode::MUL);
        else if (bin->op == "/") emit(OpCode::DIV);
        else if (bin->op == "%") emit(OpCode::MOD);
        else if (bin->op == "==") emit(OpCode::EQ);
        else if (bin->op == "!=") emit(OpCode::NE);
        else if (bin->op == "<") emit(OpCode::LT);
        else if (bin->op == "<=") emit(OpCode::LE);
        else if (bin->op == ">") emit(OpCode::GT);
        else if (bin->op == ">=") emit(OpCode::GE);
        else {
            throw std::runtime_error("Unknown binary operator: " + bin->op);
        }
        return;
    }
    
    // UnaryExpr
    if (auto* un = dynamic_cast<const UnaryExpr*>(&expr)) {
        compileExpression(*un->operand);
        
        if (un->op == "-") emit(OpCode::NEG);
        else if (un->op == "not" || un->op == "!") emit(OpCode::NOT);
        else if (un->op == "+") { /* no-op */ }
        else {
            throw std::runtime_error("Unknown unary operator: " + un->op);
        }
        return;
    }
    
    // ArrayExpr
    if (auto* arr = dynamic_cast<const ArrayExpr*>(&expr)) {
        for (const auto& elem : arr->elements) {
            compileExpression(*elem);
        }
        emit(OpCode::MAKE_ARRAY, arr->elements.size());
        return;
    }
    
    // IndexExpr
    if (auto* idx = dynamic_cast<const IndexExpr*>(&expr)) {
        compileExpression(*idx->base);
        compileExpression(*idx->index);
        emit(OpCode::ARRAY_GET);
        return;
    }

if (auto* func = dynamic_cast<const FunctionExpr*>(&expr)) {
    // 1. Создать FunctionDef
    auto fdef = std::make_shared<Interpreter::FunctionDef>();
    fdef->params = func->params;
    fdef->name = "<lambda>";
    
    // 2. Временно переключить компиляцию
    auto saved_bytecode = std::move(bytecode_);
    bytecode_.clear();
    
    for (auto& stmt : func->body) {
        compileStatement(*stmt);
    }
    // Гарантируем RETURN
    emit(OpCode::PUSH_NIL, 0);
    emit(OpCode::RETURN, 0);
    
    fdef->bytecode = std::move(bytecode_);
    bytecode_ = std::move(saved_bytecode);  // восстановить
    
    // 3. Добавить как константу
    constants_.push_back(Interpreter::Value(fdef));
    int const_idx = constants_.size() - 1;
    
    // 4. Emit
    emit(OpCode::PUSH_NUMBER, const_idx);
    return;
}

   if (auto* call = dynamic_cast<const CallExpr*>(&expr)) {
        // Проверяем на встроенные функции
        if (auto* callee_var = dynamic_cast<const VariableExpr*>(call->callee.get())) {
            const std::string& func_name = callee_var->name;
            
            // Список встроенных функций
            static const std::unordered_set<std::string> builtins = {
                "to_string", "len", "push", "join", "range", 
                "floor", "sqrt", "sort", "println"
            };
            
            if (builtins.count(func_name)) {
                // Компилируем аргументы
                for (const auto& arg : call->args) {
                    compileExpression(*arg);
                }
                
                // Вызываем встроенную функцию
                emit(OpCode::CALL_BUILTIN, addVariableName(func_name));
                return;
            }
        }
        
        // Пользовательские функции - пока заглушка
        // Компилируем аргументы
        for (const auto& arg : call->args) {
            compileExpression(*arg);
        }
        
        // Компилируем callee
        compileExpression(*call->callee);
        
        // Вызываем
        emit(OpCode::CALL, call->args.size());
        return;
    }
    
    throw std::runtime_error("Unsupported expression type");
}

void Compiler::emit(OpCode opcode, int32_t operand) {
    bytecode_.emplace_back(opcode, operand);
}

int32_t Compiler::addConstant(const Value& val) {
    constants_.push_back(val);
    return constants_.size() - 1;
}

int32_t Compiler::addVariableName(const std::string& name) {
    if (auto it = variable_indices_.find(name); it != variable_indices_.end()) {
        return it->second;
    }
    
    int32_t idx = variable_names_.size();
    variable_names_.push_back(name);
    variable_indices_[name] = idx;
    return idx;
}

} // namespace BaldVM

