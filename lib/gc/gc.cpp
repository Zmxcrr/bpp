#include "gc.h"

namespace BaldVM {
namespace GC {

MarkSweepGC::MarkSweepGC(std::stack<Frame>& frames, std::vector<Interpreter::Value>& stack)
    : frames_(frames), stack_(stack) {}

void MarkSweepGC::collect() {
    std::unordered_set<const Interpreter::Value*> reachable;
    
    // Mark phase: from value stack
    for (const auto& val : stack_) {
        mark(&val, reachable);
    }
    
    // Mark phase: from all frames
    auto frames_copy = frames_;
    while (!frames_copy.empty()) {
        auto& frame = frames_copy.top();
        
        for (auto& [name, val] : frame.locals) {
            mark(&val, reachable);
        }
        
        frames_copy.pop();
    }
    
    // Sweep phase: remove unreachable objects
}

void MarkSweepGC::mark(const Interpreter::Value* val, 
                       std::unordered_set<const Interpreter::Value*>& reachable) {
    if (!val || !reachable.insert(val).second) return;
    
    // Recursively mark array elements
    if (val->type == Interpreter::ValueType::ARRAY && val->arrayValue) {
        for (const auto& elem : *val->arrayValue) {
            mark(&elem, reachable);
        }
    }
}

} // namespace GC
} // namespace BaldVM

