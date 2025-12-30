#pragma once
#include <cstdint>
#include <string>

namespace BaldVM {

enum class OpCode : uint8_t {
    // Stack operations
    PUSH_NUMBER,      // Push constant number onto stack
    PUSH_STRING,      // Push constant string onto stack
    PUSH_BOOL,        // Push boolean onto stack
    PUSH_NIL,         // Push nil onto stack
    LOAD_VAR,         // Load variable onto stack
    STORE_VAR,        // Pop value and store in variable
    POP,              // Pop top of stack
    DUP,	      // Duplicate top of stack
    
    // Arithmetic
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    NEG,              // Unary minus
    
    // Comparison
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,
    
    // Logical
    AND,
    OR,
    NOT,
    
    // Control flow
    JUMP,             // Unconditional jump
    JUMP_IF_FALSE,    // Jump if top of stack is false
    JUMP_IF_TRUE,     // Jump if top of stack is true
    
    // Functions
    CALL,             // Call function
    RETURN,           // Return from function
    CALL_BUILTIN,
    MAKE_FUNCTION,
    CALL_FUNCTION,
    
    // Arrays
    MAKE_ARRAY,       // Create array from N stack values
    ARRAY_GET,        // Array access: arr[index]
    ARRAY_SET,        // Array set: arr[index] = value
    ARRAY_LEN,        // Get array length
    
    // Built-ins
    PRINT,
    
    // Special
    HALT              // Stop execution
};

struct Instruction {
    OpCode opcode;
    int32_t operand;     // Can be: constant index, variable index, jump offset
    
    Instruction(OpCode op = OpCode::HALT, int32_t arg = 0) 
        : opcode(op), operand(arg) {}
};

std::string opcodeToString(OpCode op);

} // namespace BaldVM

