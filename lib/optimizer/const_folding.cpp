#include "const_folding.h"
#include <cmath>

namespace BaldVM {
namespace Optimizer {

using namespace Interpreter;

void ConstantFoldingOptimizer::optimize(std::vector<Instruction>& bytecode,
                                       std::vector<Value>& constants) {
    std::vector<Instruction> optimized;
    
    for (size_t i = 0; i < bytecode.size(); ) {
        // Pattern: PUSH const1, PUSH const2, OP → PUSH result
        if (i + 2 < bytecode.size()) {
            const auto& i1 = bytecode[i];
            const auto& i2 = bytecode[i + 1];
            const auto& i3 = bytecode[i + 2];
            
            Value result;
            if (tryFold(i1, i2, i3, constants, result)) {
                // Fold successful: emit single PUSH
                int32_t const_idx = constants.size();
                constants.push_back(result);
                optimized.emplace_back(OpCode::PUSH_NUMBER, const_idx);
                i += 3;
                continue;
            }
        }
        
        // No optimization, keep instruction
        optimized.push_back(bytecode[i]);
        i++;
    }
    
    bytecode = std::move(optimized);
}

bool ConstantFoldingOptimizer::tryFold(const Instruction& i1, 
                                      const Instruction& i2, 
                                      const Instruction& i3,
                                      const std::vector<Value>& constants,
                                      Value& result) {
    // Check if i1 and i2 are constant pushes
    if (i1.opcode != OpCode::PUSH_NUMBER || i2.opcode != OpCode::PUSH_NUMBER) {
        return false;
    }
    
    if (i1.operand < 0 || i2.operand < 0 ||
        static_cast<size_t>(i1.operand) >= constants.size() ||
        static_cast<size_t>(i2.operand) >= constants.size()) {
        return false;
    }
    
    const auto& c1 = constants[i1.operand];
    const auto& c2 = constants[i2.operand];
    
    if (c1.type != ValueType::NUMBER || c2.type != ValueType::NUMBER) {
        return false;
    }
    
    double a = c1.numberValue;
    double b = c2.numberValue;
    double res = 0;
    
    switch (i3.opcode) {
        case OpCode::ADD: res = a + b; break;
        case OpCode::SUB: res = a - b; break;
        case OpCode::MUL: res = a * b; break;
        case OpCode::DIV:
            if (b == 0) return false;
            res = a / b;
            break;
        case OpCode::MOD: res = std::fmod(a, b); break;
        case OpCode::LT: res = (a < b) ? 1.0 : 0.0; break;
        case OpCode::LE: res = (a <= b) ? 1.0 : 0.0; break;
        case OpCode::GT: res = (a > b) ? 1.0 : 0.0; break;
        case OpCode::GE: res = (a >= b) ? 1.0 : 0.0; break;
        case OpCode::EQ: res = (a == b) ? 1.0 : 0.0; break;
        case OpCode::NE: res = (a != b) ? 1.0 : 0.0; break;
        default: return false;
    }
    
    result = Value(res);
    return true;
}

} // namespace Optimizer
} // namespace BaldVM

