#pragma once
#include "../vm/frame.h"
#include "../interpreter/value/value.h"
#include <vector>
#include <stack>
#include <unordered_set>

namespace BaldVM {
namespace GC {

class MarkSweepGC {
public:
    MarkSweepGC(std::stack<Frame>& frames, std::vector<Interpreter::Value>& stack);
    
    void collect();
    
private:
    void mark(const Interpreter::Value* val, std::unordered_set<const Interpreter::Value*>& reachable);
    
    std::stack<Frame>& frames_;
    std::vector<Interpreter::Value>& stack_;
};

} // namespace GC
} // namespace BaldVM

