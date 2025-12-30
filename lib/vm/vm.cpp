#include "vm.h"
#include <iostream>
#include <stdexcept>
#include <cmath>
#ifdef BALD_HAS_GC
#include "../gc/gc.h"
#endif

namespace BaldVM {

VirtualMachine::VirtualMachine() {
    stack_.reserve(256);
    frames_.push(Frame{});  // Global frame
}

VirtualMachine::~VirtualMachine() = default;


void VirtualMachine::execute(const std::vector<Instruction>& bytecode,
                             const std::vector<Interpreter::Value>& constants,
                             const std::vector<std::string>& variable_names,
                             std::ostream& out) {
    instruction_pointer_ = 0;
    
    Frame main_frame;
    main_frame.code = &bytecode;
    frames_.push(std::move(main_frame));
    
    while (instruction_pointer_ < frames_.top().code->size()) {
        const auto& instr = (*frames_.top().code)[instruction_pointer_];

        executeInstruction(instr, constants, variable_names, out);
        
        instructions_executed_++;
        stack_peak_ = std::max(stack_peak_, stack_.size());
        
        if (++instructions_since_gc_ >= GC_THRESHOLD) {
            #ifdef BALD_HAS_GC
                runGC();
            #endif
            instructions_since_gc_ = 0;
        }
        
        if (instr.opcode == OpCode::HALT) break;
        if (instr.opcode == OpCode::RETURN && frames_.size() == 1) break;
        
        instruction_pointer_++;
    }
}

void VirtualMachine::executeInstruction(const Instruction& instr,
                                       const std::vector<Interpreter::Value>& constants,
                                       const std::vector<std::string>& variable_names,
                                       std::ostream& out) {
    using namespace Interpreter;
    
    switch (instr.opcode) {
        case OpCode::PUSH_NUMBER:
        case OpCode::PUSH_STRING:
        case OpCode::PUSH_BOOL:
        case OpCode::PUSH_NIL:
            if (instr.operand < 0 || static_cast<size_t>(instr.operand) >= constants.size()) {
                throw std::runtime_error("Invalid constant index");
            }
            push(constants[instr.operand]);
            break;
            
        case OpCode::STORE_VAR: {
            if (instr.operand < 0 || static_cast<size_t>(instr.operand) >= variable_names.size()) {
                throw std::runtime_error("Invalid variable index");
            }
            const auto& var_name = variable_names[instr.operand];
            frames_.top().locals[var_name] = pop();
            break;
        }
            
        case OpCode::POP:
            pop();
            break;

	case OpCode::DUP:
	    push(peek());
	    break;


	case OpCode::ADD: {
	    auto b = pop();
	    auto a = pop();
	    
	    // number + number
	    if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
		push(Value(a.numberValue + b.numberValue));
	    }
	    // string + string
	    else if (a.type == ValueType::STRING && b.type == ValueType::STRING) {
		push(Value(*a.stringValue + *b.stringValue));
	    }
	    // string + number (автоконвертация)
	    else if (a.type == ValueType::STRING && b.type == ValueType::NUMBER) {
		push(Value(*a.stringValue + std::to_string(b.numberValue)));
	    }
	    // number + string (автоконвертация)
	    else if (a.type == ValueType::NUMBER && b.type == ValueType::STRING) {
		push(Value(std::to_string(a.numberValue) + *b.stringValue));
	    }
	    else {
		throw std::runtime_error("Type error in ADD");
	    }
	    break;
	}
            
        case OpCode::SUB: {
            auto b = pop();
            auto a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                push(Value(a.numberValue - b.numberValue));
            } else {
                throw std::runtime_error("Type error in SUB");
            }
            break;
        }
            
        case OpCode::MUL: {
            auto b = pop();
            auto a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                push(Value(a.numberValue * b.numberValue));
            } else {
                throw std::runtime_error("Type error in MUL");
            }
            break;
        }
            
        case OpCode::DIV: {
            auto b = pop();
            auto a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                if (b.numberValue == 0) throw std::runtime_error("Division by zero");
                push(Value(a.numberValue / b.numberValue));
            } else {
                throw std::runtime_error("Type error in DIV");
            }
            break;
        }
            
        case OpCode::MOD: {
            auto b = pop();
            auto a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                push(Value(std::fmod(a.numberValue, b.numberValue)));
            } else {
                throw std::runtime_error("Type error in MOD");
            }
            break;
        }
            
        case OpCode::NEG: {
            auto a = pop();
            if (a.type == ValueType::NUMBER) {
                push(Value(-a.numberValue));
            } else {
                throw std::runtime_error("Type error in NEG");
            }
            break;
        }
            
        case OpCode::EQ: {
            auto b = pop();
            auto a = pop();
            bool result = false;
            if (a.type == b.type) {
                switch (a.type) {
                    case ValueType::NUMBER: result = (a.numberValue == b.numberValue); break;
                    case ValueType::BOOL: result = (a.boolValue == b.boolValue); break;
                    case ValueType::STRING: result = (*a.stringValue == *b.stringValue); break;
                    case ValueType::NIL: result = true; break;
                    default: break;
                }
            }
            push(Value(result));
            break;
        }
            
        case OpCode::NE: {
            auto b = pop();
            auto a = pop();
            bool result = true;
            if (a.type == b.type) {
                switch (a.type) {
                    case ValueType::NUMBER: result = (a.numberValue != b.numberValue); break;
                    case ValueType::BOOL: result = (a.boolValue != b.boolValue); break;
                    case ValueType::STRING: result = (*a.stringValue != *b.stringValue); break;
                    case ValueType::NIL: result = false; break;
                    default: break;
                }
            }
            push(Value(result));
            break;
        }
            
        case OpCode::LT: {
            auto b = pop();
            auto a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                push(Value(a.numberValue < b.numberValue));
            } else {
                push(Value(false));
            }
            break;
        }
            
        case OpCode::LE: {
            auto b = pop();
            auto a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                push(Value(a.numberValue <= b.numberValue));
            } else {
                push(Value(false));
            }
            break;
        }
            
        case OpCode::GT: {
            auto b = pop();
            auto a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                push(Value(a.numberValue > b.numberValue));
            } else {
                push(Value(false));
            }
            break;
        }
            
        case OpCode::GE: {
            auto b = pop();
            auto a = pop();
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                push(Value(a.numberValue >= b.numberValue));
            } else {
                push(Value(false));
            }
            break;
        }
            
        case OpCode::AND: {
            auto b = pop();
            auto a = pop();
            push(Value(a.toBool() && b.toBool()));
            break;
        }
            
        case OpCode::OR: {
            auto b = pop();
            auto a = pop();
            push(Value(a.toBool() || b.toBool()));
            break;
        }
            
        case OpCode::NOT: {
            auto a = pop();
            push(Value(!a.toBool()));
            break;
        }
            
        case OpCode::JUMP:
            instruction_pointer_ = instr.operand - 1;  // -1 because we'll ++ after
            break;
            
        case OpCode::JUMP_IF_FALSE:
            if (!pop().toBool()) {
                instruction_pointer_ = instr.operand - 1;
            }
            break;
            
        case OpCode::JUMP_IF_TRUE:
            if (pop().toBool()) {
                instruction_pointer_ = instr.operand - 1;
            }
            break;
            
        case OpCode::MAKE_ARRAY: {
            size_t size = instr.operand;
            std::vector<Value> elements(size);
            for (int i = size - 1; i >= 0; --i) {
                elements[i] = pop();
            }
            push(Value(std::move(elements)));
            break;
        }
            
        case OpCode::ARRAY_GET: {
            auto index = pop();
            auto array = pop();
            if (array.type == ValueType::ARRAY && index.type == ValueType::NUMBER) {
                size_t idx = static_cast<size_t>(index.numberValue);
                if (idx < array.arrayValue->size()) {
                    push((*array.arrayValue)[idx]);
                } else {
                    throw std::runtime_error("Array index out of bounds");
                }
            } else {
                throw std::runtime_error("Type error in ARRAY_GET");
            }
            break;
        }

	case OpCode::ARRAY_LEN: {
	    auto arr = pop();
	    if (arr.type == ValueType::ARRAY) {
		push(Value(static_cast<double>(arr.arrayValue->size())));
	    } else if (arr.type == ValueType::STRING) {
		push(Value(static_cast<double>(arr.stringValue->size())));
	    } else {
		throw std::runtime_error("ARRAY_LEN: not an array or string");
	    }
	    break;
	}

	case OpCode::ARRAY_SET: {
	    auto value = pop();
	    auto index = pop();
	    auto array = pop();
	    
	    if (array.type == ValueType::ARRAY && index.type == ValueType::NUMBER) {
		size_t idx = static_cast<size_t>(index.numberValue);
		if (idx < array.arrayValue->size()) {
		    (*array.arrayValue)[idx] = value;
		} else {
		    throw std::runtime_error("ARRAY_SET: index out of bounds");
		}
	    } else {
		throw std::runtime_error("ARRAY_SET: type error");
	    }
	    break;
	}
            
        case OpCode::PRINT:
            out << peek().toString() << std::endl;
            break;
            
        case OpCode::HALT:
	case OpCode::RETURN: {
	    Value ret;
	    if (!stack_.empty()) {
		ret = pop();
	    } else {
		ret = Value();
	    }
	    
	    if (frames_.size() <= 1) {
		return;
	    }
	    
	    size_t ret_ip = frames_.top().return_ip;
	    frames_.pop();
	    instruction_pointer_ = ret_ip;
	    push(ret);
	    break;
	}
	case OpCode::CALL: {
	    size_t argc = instr.operand;
	    
	    auto callee = pop();
	    if (callee.type != ValueType::FUNCTION || !callee.functionValue) {
		throw std::runtime_error("CALL: not a function");
	    }
	    
	    auto& fdef = *callee.functionValue;
	    if (fdef.params.size() != argc) {
		throw std::runtime_error("CALL: arity mismatch");
	    }
	    
	    std::vector<Value> args(argc);
	    for (int i = argc - 1; i >= 0; --i) args[i] = pop();
	    
	    Frame new_frame;
	    new_frame.function = callee.functionValue;
	    new_frame.return_ip = instruction_pointer_;
	    new_frame.code = &fdef.bytecode;
	    for (size_t i = 0; i < argc; ++i) {
		new_frame.locals[fdef.params[i]] = args[i];
	    }
	    
	    frames_.push(std::move(new_frame));
	    instruction_pointer_ = -1;  // начнём с 0 после ++
	    break;
	}
	case OpCode::CALL_BUILTIN: {
		    if (instr.operand < 0 || static_cast<size_t>(instr.operand) >= variable_names.size()) {
			throw std::runtime_error("Invalid builtin name index");
		    }
		    
		    const std::string& func_name = variable_names[instr.operand];
		    
		    // to_string
		    if (func_name == "to_string") {
			auto val = pop();
			push(Value(val.toString()));
		    }
		    // len
		    else if (func_name == "len") {
			auto val = pop();
			if (val.type == ValueType::ARRAY) {
			    push(Value(static_cast<double>(val.arrayValue->size())));
			} else if (val.type == ValueType::STRING) {
			    push(Value(static_cast<double>(val.stringValue->size())));
			} else {
			    push(Value(0.0));
			}
		    }
		    // push
		    else if (func_name == "push") {
			auto elem = pop();
			auto arr = pop();
			if (arr.type == ValueType::ARRAY) {
			    arr.arrayValue->push_back(elem);
			    push(arr);
			} else {
			    throw std::runtime_error("push: first argument must be array");
			}
		    }
		    // join
		    else if (func_name == "join") {
			auto sep = pop();
			auto arr = pop();
			if (arr.type != ValueType::ARRAY || sep.type != ValueType::STRING) {
			    throw std::runtime_error("join: invalid arguments");
			}
			std::string result;
			for (size_t i = 0; i < arr.arrayValue->size(); ++i) {
			    if (i > 0) result += *sep.stringValue;
			    result += (*arr.arrayValue)[i].toString();
			}
			push(Value(result));
		    }
		    // range
		    else if (func_name == "range") {
			// Проверяем сколько аргументов (они уже на стеке)
		    // Для range нужно знать количество - берём из размера стека
		    // Упрощённо: всегда берём 3 аргумента и используем defaults
			    
		    // Безопасно: создаём range(0, N) всегда
		    auto arg = pop();
		    if (arg.type != ValueType::NUMBER) {
			throw std::runtime_error("range: argument must be number");
		    }
		    
		    std::vector<Value> arr;
		    for (double i = 0; i < arg.numberValue; i += 1.0) {
			arr.push_back(Value(i));
		    }
		    push(Value(std::move(arr)));
		}
		    // floor
		    else if (func_name == "floor") {
			auto val = pop();
			if (val.type == ValueType::NUMBER) {
			    push(Value(std::floor(val.numberValue)));
			} else {
			    push(Value(0.0));
			}
		    }
		    // sqrt
		    else if (func_name == "sqrt") {
			auto val = pop();
			if (val.type == ValueType::NUMBER) {
			    push(Value(std::sqrt(val.numberValue)));
			} else {
			    push(Value(0.0));
			}
		    }
		    // sort
		    else if (func_name == "sort") {
			auto arr = pop();
			if (arr.type == ValueType::ARRAY) {
			    std::sort(arr.arrayValue->begin(), arr.arrayValue->end(), 
				     [](const Value& a, const Value& b) {
				if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
				    return a.numberValue < b.numberValue;
				}
				return false;
			    });
			    push(arr);
			} else {
			    throw std::runtime_error("sort: argument must be array");
			}
		    }
		    else {
			throw std::runtime_error("Unknown builtin function: " + func_name);
		    }
		    break;
		}

	case OpCode::MAKE_FUNCTION: {
	    // operand = index в constants, там лежит готовый FunctionDef
	    if (instr.operand >= 0 && static_cast<size_t>(instr.operand) < constants.size()) {
		push(constants[instr.operand]);
	    } else {
		throw std::runtime_error("MAKE_FUNCTION: invalid constant index");
	    }
	    break;
	}

	case OpCode::LOAD_VAR: {
	    const auto& var_name = variable_names[instr.operand];
	    
	    // Сначала locals текущего frame
	    if (!frames_.empty() && frames_.top().locals.count(var_name)) {
		push(frames_.top().locals[var_name]);
		break;
	    }
	    
	    // Потом ищем в остальных frames (globals)
	    auto frames_copy = frames_;
	    bool found = false;
	    while (!frames_copy.empty()) {
		if (frames_copy.top().locals.count(var_name)) {
		    push(frames_copy.top().locals[var_name]);
		    found = true;
		    break;
		}
		frames_copy.pop();
	    }
	    if (!found) {
		throw std::runtime_error("Undefined variable: " + var_name);
	    }
	    break;
	}
        default:
	    auto raw = static_cast<int>(instr.opcode);
	    std::cerr << "Unknown opcode: " << raw << "\n";
            throw std::runtime_error("Unknown opcode");

    }
}

void VirtualMachine::push(const Interpreter::Value& val) {
    stack_.push_back(val);
}


Interpreter::Value VirtualMachine::pop() {
    if (stack_.empty()) {
        std::cerr << "Stack underflow at IP=" << instruction_pointer_ 
                  << " frame_depth=" << frames_.size() << std::endl;
        throw std::runtime_error("Stack underflow");
    }
    auto val = stack_.back();
    stack_.pop_back();
    return val;
}

Interpreter::Value& VirtualMachine::peek() {
    if (stack_.empty()) {
        throw std::runtime_error("Empty stack");
    }
    return stack_.back();
}

void VirtualMachine::runGC() {
    #ifdef BALD_HAS_GC
        GC::MarkSweepGC gc(frames_, stack_);
        gc.collect();
        gc_collections_++;
    #endif
}

std::string opcodeToString(OpCode op) {
    switch (op) {
        case OpCode::PUSH_NUMBER: return "PUSH_NUMBER";
        case OpCode::PUSH_STRING: return "PUSH_STRING";
        case OpCode::PUSH_BOOL: return "PUSH_BOOL";
        case OpCode::PUSH_NIL: return "PUSH_NIL";
        case OpCode::LOAD_VAR: return "LOAD_VAR";
        case OpCode::STORE_VAR: return "STORE_VAR";
        case OpCode::POP: return "POP";
	case OpCode::DUP: return "DUP";
        case OpCode::ADD: return "ADD";
        case OpCode::SUB: return "SUB";
        case OpCode::MUL: return "MUL";
        case OpCode::DIV: return "DIV";
        case OpCode::MOD: return "MOD";
        case OpCode::NEG: return "NEG";
        case OpCode::EQ: return "EQ";
        case OpCode::NE: return "NE";
        case OpCode::LT: return "LT";
        case OpCode::LE: return "LE";
        case OpCode::GT: return "GT";
        case OpCode::GE: return "GE";
        case OpCode::AND: return "AND";
        case OpCode::OR: return "OR";
        case OpCode::NOT: return "NOT";
        case OpCode::JUMP: return "JUMP";
        case OpCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case OpCode::JUMP_IF_TRUE: return "JUMP_IF_TRUE";
        case OpCode::MAKE_ARRAY: return "MAKE_ARRAY";
        case OpCode::ARRAY_GET: return "ARRAY_GET";
        case OpCode::PRINT: return "PRINT";
        case OpCode::HALT: return "HALT";
        case OpCode::RETURN: return "RETURN";
        case OpCode::CALL_BUILTIN: return "CALL_BUILTIN";
        case OpCode::MAKE_FUNCTION: return "MAKE_FUNCTION";
        case OpCode::CALL_FUNCTION: return "CALL_FUNCTION";
        default: return "UNKNOWN";
    }
}

} // namespace BaldVM

