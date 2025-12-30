#include "dead_code.h"
#include <vector>

namespace BaldVM {
namespace Optimizer {

void DeadCodeOptimizer::optimize(std::vector<Instruction>& bytecode,
                                std::vector<Interpreter::Value>&) {
    auto reachable = findReachable(bytecode);
    
    std::vector<Instruction> optimized;
    for (size_t i = 0; i < bytecode.size(); ++i) {
        if (reachable.count(i)) {
            optimized.push_back(bytecode[i]);
        }
    }
    
    bytecode = std::move(optimized);
}

std::unordered_set<size_t> DeadCodeOptimizer::findReachable(
    const std::vector<Instruction>& bytecode) {
    
    std::unordered_set<size_t> reachable;
    std::vector<size_t> worklist = {0};  // Start from instruction 0
    
    while (!worklist.empty()) {
        size_t idx = worklist.back();
        worklist.pop_back();
        
        if (reachable.count(idx) || idx >= bytecode.size()) {
            continue;
        }
        
        reachable.insert(idx);
        
        const auto& instr = bytecode[idx];
        
        // Terminal instructions
        if (instr.opcode == OpCode::HALT || instr.opcode == OpCode::RETURN) {
            continue;
        }
        
        // Unconditional jump
        if (instr.opcode == OpCode::JUMP) {
            if (instr.operand >= 0 && static_cast<size_t>(instr.operand) < bytecode.size()) {
                worklist.push_back(instr.operand);
            }
            continue;
        }
        
        // Conditional jump - both paths reachable
        if (instr.opcode == OpCode::JUMP_IF_FALSE || 
            instr.opcode == OpCode::JUMP_IF_TRUE) {
            if (instr.operand >= 0 && static_cast<size_t>(instr.operand) < bytecode.size()) {
                worklist.push_back(instr.operand);
            }
            worklist.push_back(idx + 1);
            continue;
        }
        
        // Normal instruction - fall through to next
        worklist.push_back(idx + 1);
    }
    
    return reachable;
}

} // namespace Optimizer
} // namespace BaldVM

