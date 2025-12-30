#pragma once
#include "optimizer.h"
#include <unordered_set>

namespace BaldVM {
namespace Optimizer {

class DeadCodeOptimizer : public IOptimizer {
public:
    void optimize(std::vector<Instruction>& bytecode,
                 std::vector<Interpreter::Value>& constants) override;
                 
private:
    std::unordered_set<size_t> findReachable(const std::vector<Instruction>& bytecode);
};

} // namespace Optimizer
} // namespace BaldVM

