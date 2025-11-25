#pragma once

#include <memory>
#include <vector>
#include <string>
#include <stdexcept>

namespace Interpreter {

    enum class ValueType { NIL, NUMBER, BOOL, STRING, FUNCTION, ARRAY };

    struct FunctionDef;

    struct Value {
        ValueType type{ValueType::NIL};
        double numberValue{0.0};
        bool boolValue{false};
        std::shared_ptr<std::string> stringValue;
        std::shared_ptr<std::vector<Value> > arrayValue;
        std::shared_ptr<FunctionDef> functionValue;

        Value& operator=(const Value& other);

        Value();

        Value(const Value& other);

        explicit Value(bool b);

        explicit Value(double num);

        explicit Value(std::shared_ptr<FunctionDef> func);

        explicit Value(std::string s);

        explicit Value(std::vector<Value>&& arr);

        [[nodiscard]] bool toBool() const;
    };

    inline std::string valueTypeToString(ValueType type) {
        switch (type) {
            case ValueType::NIL: return "nil";
            case ValueType::NUMBER: return "number";
            case ValueType::BOOL: return "bool";
            case ValueType::STRING: return "string";
            case ValueType::FUNCTION: return "function";
            case ValueType::ARRAY: return "array";
            default: return "<unknown_type>";
        }
    }

    inline Value::Value() = default;

    inline Value::Value(const Value &other): type(other.type),
                                             numberValue(other.numberValue),
                                             boolValue(other.boolValue),
                                             stringValue(other.stringValue),
                                             arrayValue(other.arrayValue), 
                                             functionValue(other.functionValue) {
    }

    inline Value &Value::operator=(const Value &other) {
        if (this == &other) return *this;
        type = other.type;
        numberValue = other.numberValue;
        boolValue = other.boolValue;
        stringValue = other.stringValue;
        arrayValue = other.arrayValue; 
        functionValue = other.functionValue;
        return *this;
    }

    inline Value::Value(bool b): type(ValueType::BOOL), boolValue(b) {
    }

    inline Value::Value(double num): type(ValueType::NUMBER), numberValue(num) {
    }

    inline Value::Value(std::shared_ptr<FunctionDef> func): type(ValueType::FUNCTION), functionValue(std::move(func)) {
    }

    inline Value::Value(std::string s): type(ValueType::STRING),
                                        stringValue(std::make_shared<std::string>(std::move(s))) {
    }

    inline Value::Value(std::vector<Value> &&arr): type(ValueType::ARRAY),
                                                   arrayValue(std::make_shared<std::vector<Value> >(std::move(arr))) {
    }

    inline bool Value::toBool() const {
        switch (type) {
            case ValueType::BOOL: return boolValue;
            case ValueType::NUMBER: return numberValue != 0.0;
            case ValueType::NIL: return false;
            case ValueType::STRING: return !stringValue->empty();
            case ValueType::ARRAY: return !arrayValue->empty();
            case ValueType::FUNCTION: return true;
            default: throw std::runtime_error(
                    "Невозможно преобразовать тип в булево значение: " + valueTypeToString(type));
        }
    }

} //Interpreter