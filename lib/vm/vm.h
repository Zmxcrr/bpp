#pragma once
#include "bytecode.h"
#include "frame.h"
#include "../interpreter/value/value.h"
#include <vector>
#include <stack>
#include <unordered_map>
#include <memory>

namespace BaldVM {

class VirtualMachine {
public:
    VirtualMachine();
    ~VirtualMachine();
    
    // Execute bytecode
    void execute(const std::vector<Instruction>& bytecode,
                 const std::vector<Interpreter::Value>& constants,
                 const std::vector<std::string>& variable_names,
                 std::ostream& out);
    
    // Statistics
    size_t getInstructionsExecuted() const { return instructions_executed_; }
    size_t getStackPeak() const { return stack_peak_; }
    size_t getGCCollections() const { return gc_collections_; }
    
private:
    void executeInstruction(const Instruction& instr,
                           const std::vector<Interpreter::Value>& constants,
                           const std::vector<std::string>& variable_names,
                           std::ostream& out);
    
    void push(const Interpreter::Value& val);
    Interpreter::Value pop();
    Interpreter::Value& peek();
    
    void runGC();
    
    // VM state
    std::vector<Interpreter::Value> stack_;
    std::stack<Frame> frames_;
    size_t instruction_pointer_ = 0;
    
    // Statistics
    size_t instructions_executed_ = 0;
    size_t stack_peak_ = 0;
    size_t gc_collections_ = 0;
    size_t instructions_since_gc_ = 0;
    
    static constexpr size_t GC_THRESHOLD = 500;
};

} // namespace BaldVM

