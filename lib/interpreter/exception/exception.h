#pragma once

#include "value.h"

namespace Interpreter {
    struct ReturnException : public std::exception {
        Value value;

        explicit ReturnException(const Value&);
    };

    struct BreakException : public std::exception {
        [[nodiscard]] const char* what() const noexcept override;
    };

    struct ContinueException : public std::exception {
        [[nodiscard]] const char* what() const noexcept override;
    };

    inline ReturnException::ReturnException(const Value &v): value(v) {}

    inline const char *BreakException::what() const noexcept { return "Break"; }

    inline const char *ContinueException::what() const noexcept { return "Continue"; }
}