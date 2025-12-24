#ifndef GC_H
#define GC_H

#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>

namespace Interpreter {

struct Value;
struct Environment;

namespace GC {

class GCObject {
public:
    bool marked = false;
    virtual ~GCObject() = default;
    virtual void mark() = 0;
};

class GCValue : public GCObject {
public:
    Value value;

    explicit GCValue(const Value& val) : value(val) {}
    void mark() override;
};

class GCEnvironment : public GCObject {
public:
    Environment* env;

    explicit GCEnvironment(Environment* e) : env(e) {}
    ~GCEnvironment() override { delete env; }
    void mark() override;
};

class Collector {
protected:
    std::unordered_set<GCObject*> roots;

    size_t collections_count = 0;
    size_t objects_collected = 0;

    size_t collection_threshold = 100;
    size_t allocation_count = 0;

public:
    std::vector<std::unique_ptr<GCObject>> objects;

    static Collector& instance() {
        static Collector gc;
        return gc;
    }

    template<typename T, typename... Args>
    T* allocate(Args&&... args) {
        auto obj = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = obj.get();
        objects.push_back(std::move(obj));

        allocation_count++;
        if (allocation_count >= collection_threshold) {
            collect();
        }

        return ptr;
    }

    void addRoot(GCObject* obj) {
        roots.insert(obj);
    }

    void removeRoot(GCObject* obj) {
        roots.erase(obj);
    }

    void collect() {
        for (auto* root : roots) {
            if (root) {
                markObject(root);
            }
        }

        size_t before_count = objects.size();
        objects.erase(
            std::remove_if(objects.begin(), objects.end(),
                [](const std::unique_ptr<GCObject>& obj) {
                    return !obj->marked;
                }),
            objects.end()
        );

        for (auto& obj : objects) {
            obj->marked = false;
        }

        size_t collected = before_count - objects.size();
        objects_collected += collected;
        collections_count++;
        allocation_count = 0;
    }

    struct Stats {
        size_t total_objects;
        size_t collections_count;
        size_t objects_collected;
        size_t roots_count;
    };

    Stats getStats() const {
        return {
            objects.size(),
            collections_count,
            objects_collected,
            roots.size()
        };
    }

    void setThreshold(size_t threshold) {
        collection_threshold = threshold;
    }

    void clear() {
        objects.clear();
        roots.clear();
        collections_count = 0;
        objects_collected = 0;
        allocation_count = 0;
    }

    void markObject(GCObject* obj) {
        if (!obj || obj->marked) return;
        obj->marked = true;
        obj->mark();
    }
};

template<typename T, typename... Args>
T* allocate(Args&&... args) {
    return Collector::instance().allocate<T>(std::forward<Args>(args)...);
}

inline void collect() {
    Collector::instance().collect();
}

inline void addRoot(GCObject* obj) {
    Collector::instance().addRoot(obj);
}

inline void removeRoot(GCObject* obj) {
    Collector::instance().removeRoot(obj);
}

inline Collector::Stats getStats() {
    return Collector::instance().getStats();
}

class ScopedRoot {
private:
    GCObject* obj_;

public:
    explicit ScopedRoot(GCObject* obj) : obj_(obj) {
        if (obj_) addRoot(obj_);
    }

    ~ScopedRoot() {
        if (obj_) removeRoot(obj_);
    }

    ScopedRoot(const ScopedRoot&) = delete;
    ScopedRoot& operator=(const ScopedRoot&) = delete;

    ScopedRoot(ScopedRoot&& other) noexcept : obj_(other.obj_) {
        other.obj_ = nullptr;
    }

    ScopedRoot& operator=(ScopedRoot&& other) noexcept {
        if (this != &other) {
            if (obj_) removeRoot(obj_);
            obj_ = other.obj_;
            other.obj_ = nullptr;
        }
        return *this;
    }
};

} // namespace GC
} // namespace Interpreter

namespace Interpreter {
namespace GC {

inline void GCValue::mark() {
    switch (value.type) {
        case ValueType::ARRAY:
            if (value.arrayValue) {
                for (auto& elem : *value.arrayValue) {
                    if (elem.type == ValueType::ARRAY || 
                        elem.type == ValueType::FUNCTION ||
                        elem.type == ValueType::STRING) {
                        GCValue temp_gc(elem);
                        temp_gc.mark();
                    }
                }
            }
            break;

        case ValueType::FUNCTION:
            break;

        case ValueType::STRING:
            break;

        default:
            break;
    }
}

inline void GCEnvironment::mark() {
    if (!env) return;

    for (auto& [name, val] : env->variables) {
        if (val.type == ValueType::ARRAY || 
            val.type == ValueType::FUNCTION ||
            val.type == ValueType::STRING) {
            GCValue temp_gc(val);
            temp_gc.mark();
        }
    }

    if (env->parent) {
        for (auto& obj : Collector::instance().objects) {
            if (auto* gc_env = dynamic_cast<GCEnvironment*>(obj.get())) {
                if (gc_env->env == env->parent) {
                    Collector::instance().markObject(gc_env);
                    break;
                }
            }
        }
    }
}

} // namespace GC
} // namespace Interpreter

#endif // GC_H
