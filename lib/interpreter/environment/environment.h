#pragma once

#include "../value/value.h"
#include <map>

namespace Interpreter {
    struct Environment {
        Environment* parent{nullptr};
        Environment* env{this};
        std::map<std::string, Value> variables;

        Environment();

        explicit Environment(Environment* parentEnv);

        Environment* getGlobal();

        Environment& set_and_get_ref(const std::string& name, const Value& value);

        void set(const std::string& name, const Value& value);

        [[nodiscard]] Value get(const std::string&) const;
    };

    inline Environment::Environment() = default;

    inline Environment::Environment(Environment *parentEnv): parent(parentEnv) {
    }

    inline Environment *Environment::getGlobal() {
        Environment *e = this;
        while (e->parent) e = e->parent;
        return e;
    }

    inline Environment &Environment::set_and_get_ref(const std::string &name, const Value &value) {
        variables[name] = value;
        return *this;
    }

    inline void Environment::set(const std::string &name, const Value &value) {
        Environment *target = this;
        while (target) {
            auto it = target->variables.find(name);
            if (it != target->variables.end()) {
                target->variables[name] = value;
                return;
            }
            target = target->parent;
        }
        variables[name] = value;
    }

    inline Value Environment::get(const std::string& name) const {
            for (const Environment* currentEnv = this; currentEnv; currentEnv = currentEnv->parent) {
                    if (auto it = currentEnv->variables.find(name); it != currentEnv->variables.end()) {
                            const auto& [foundName, foundValue] = *it;
                            return foundValue;
                    }
            }
            throw std::runtime_error("Неопределённая переменная: '" + name + "'");
    }

} //Interpreter
