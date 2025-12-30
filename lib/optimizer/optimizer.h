#pragma once
#include "../vm/bytecode.h"
#include "../interpreter/value/value.h"
#include <vector>

namespace BaldVM {
namespace Optimizer {

class IOptimizer {
public:
    virtual ~IOptimizer() = default;
    
    virtual void optimize(std::vector<Instruction>& bytecode,
                         std::vector<Interpreter::Value>& constants) = 0;
};

} // namespace Optimizer
} // namespace BaldVM

