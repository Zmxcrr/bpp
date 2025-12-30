#pragma once
#include "optimizer.h"

namespace BaldVM {
namespace Optimizer {

class ConstantFoldingOptimizer : public IOptimizer {
public:
    void optimize(std::vector<Instruction>& bytecode,
                 std::vector<Interpreter::Value>& constants) override;
                 
private:
    bool tryFold(const Instruction& i1, const Instruction& i2, const Instruction& i3,
                const std::vector<Interpreter::Value>& constants,
                Interpreter::Value& result);
};

} // namespace Optimizer
} // namespace BaldVM

