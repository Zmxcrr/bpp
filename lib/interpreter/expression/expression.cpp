#include "expression.h"
#include "statement.h"
#include "value.h"
#include "environment.h"
#include "interpreter.h"
#include "functiondef.h"
#include "exception.h"
#include <cmath>
#include <random>
#include <iostream>
#include <algorithm>

namespace Interpreter {
     Expr::~Expr() = default;

     Value Expr::eval(Environment*, std::ostream&) {
		return {};
    }

    auto BinaryExpr::eval(Environment* env, std::ostream& out) -> Value {
            const Value a = left->eval(env, out);

            if (op == "and") {
                    if (!a.toBool()) {
                            return Value(false);
                    }
                    const Value b = right->eval(env, out);
                    return Value(b.toBool());
            }

            if (op == "or") {
                    if (a.toBool()) {
                            return Value(true);
                    }
                    const Value b = right->eval(env, out);
                    return Value(b.toBool());
            }

            const Value b = right->eval(env, out);

            auto equalArrays = [&](const std::vector<Value>& arr1, const std::vector<Value>& arr2) -> bool {
                    if (arr1.size() != arr2.size()) {
                            return false;
                    }
                    for (size_t i = 0; i < arr1.size(); ++i) {
                            Environment tempEnv(env);
                            const std::string tempVarA = "__comp_a_" + std::to_string(i);
                            const std::string tempVarB = "__comp_b_" + std::to_string(i);
                            tempEnv.set(tempVarA, arr1[i]);
                            tempEnv.set(tempVarB, arr2[i]);
                            Value eqResult = BinaryExpr(
                                    std::make_unique<VariableExpr>(tempVarA), "==",
                                    std::make_unique<VariableExpr>(tempVarB)
                            ).eval(&tempEnv, out);
                            if (!eqResult.toBool()) {
                                    return false;
                            }
                    }
                    return true;
            };

            auto repeatString = [](const std::string& str, int times) -> std::string {
                    times = std::max(0, times);
                    std::string result;
                    if (times > 0) {
                            result.reserve(str.size() * static_cast<size_t>(times));
                    }
                    for (int i = 0; i < times; ++i) {
                            result += str;
                    }
                    return result;
            };

            auto repeatArray = [](const std::shared_ptr<std::vector<Value>>& arr, int times) -> Value {
                    times = std::max(0, times);
                    std::vector<Value> result;
                    if (times > 0) {
                            result.reserve(arr->size() * static_cast<size_t>(times));
                    }
                    for (int i = 0; i < times; ++i) {
                            result.insert(result.end(), arr->begin(), arr->end());
                    }
                    return Value(std::move(result));
            };

            if (op == "+") {
                    if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                            return Value(a.numberValue + b.numberValue);
                    }
                    if (a.type == ValueType::STRING && b.type == ValueType::STRING) {
                            return Value(*a.stringValue + *b.stringValue);
                    }
                    if (a.type == ValueType::ARRAY && b.type == ValueType::ARRAY) {
                            std::vector<Value> result = *a.arrayValue;
                            result.insert(result.end(), b.arrayValue->begin(), b.arrayValue->end());
                            return Value(std::move(result));
                    }
                    throw std::runtime_error("Оператор '+' поддерживает только числа, строки или списки одного типа");

            } else if (op == "-") {
                    if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                            return Value(a.numberValue - b.numberValue);
                    }
                    if (a.type == ValueType::STRING && b.type == ValueType::STRING) {
                            if (a.stringValue->size() >= b.stringValue->size() && 
                                    a.stringValue->compare(a.stringValue->size() - b.stringValue->size(),
                                                                               b.stringValue->size(), *b.stringValue) == 0) {
                                    return Value(a.stringValue->substr(0, a.stringValue->size() - b.stringValue->size()));
                            }
                            return Value(*a.stringValue);
                    }
                    throw std::runtime_error("Оператор '-' поддерживает только числа или подходящие строки");

            } else if (op == "*") {
                    if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                            return Value(a.numberValue * b.numberValue);
                    }
                    if (a.type == ValueType::STRING && b.type == ValueType::NUMBER) {
                            return Value(repeatString(*a.stringValue, static_cast<int>(std::floor(b.numberValue))));
                    }
                    if (a.type == ValueType::STRING && b.type == ValueType::BOOL) {
                            return Value(repeatString(*a.stringValue, b.boolValue ? 1 : 0));
                    }
                    if (a.type == ValueType::NUMBER && b.type == ValueType::STRING) {
                            return Value(repeatString(*b.stringValue, static_cast<int>(std::floor(a.numberValue))));
                    }
                    if (a.type == ValueType::ARRAY && b.type == ValueType::NUMBER) {
                            return repeatArray(a.arrayValue, static_cast<int>(std::floor(b.numberValue)));
                    }
                    if (a.type == ValueType::NUMBER && b.type == ValueType::ARRAY) {
                            return repeatArray(b.arrayValue, static_cast<int>(std::floor(a.numberValue)));
                    }
                    throw std::runtime_error("Оператор '*' не поддерживается для данных типов (" + 
                                                                    valueTypeToString(a.type) + ", " + valueTypeToString(b.type) + ")");

            } else if (op == "/") {
                    if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                            if (b.numberValue == 0) {
                                    throw std::runtime_error("Деление на ноль");
                            }
                            return Value(a.numberValue / b.numberValue);
                    }
                    throw std::runtime_error("Оператор '/' поддерживает только числа");

            } else if (op == "%") {
                    if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                            if (b.numberValue == 0) {
                                    throw std::runtime_error("Деление на ноль в операции '%'");
                            }
                            return Value(std::fmod(a.numberValue, b.numberValue));
                    }
                    throw std::runtime_error("Оператор '%' поддерживает только числа");

            } else if (op == "^") {
                    if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                            return Value(std::pow(a.numberValue, b.numberValue));
                    }
                    throw std::runtime_error("Оператор '^' поддерживает только числа");

            } else if (op == "==") {
                    if (a.type != b.type) {
                            return Value(false);
                    }
                    switch (a.type) {
                            case ValueType::NUMBER: {
                                    return Value(a.numberValue == b.numberValue);
                            }
                            case ValueType::BOOL: {
                                    return Value(a.boolValue == b.boolValue);
                            }
                            case ValueType::STRING: {
                                    return Value(*a.stringValue == *b.stringValue);
                            }
                            case ValueType::ARRAY: {
                                    return Value(equalArrays(*a.arrayValue, *b.arrayValue));
                            }
                            case ValueType::NIL: {
                                    return Value(true);
                            }
                            case ValueType::FUNCTION: {
                                    return Value(a.functionValue == b.functionValue);
                            }
                            default: {
                                    return Value(false);
                            }
                    }

            } else if (op == "!=") {
                    if (a.type != b.type) {
                            return Value(true);
                    }
                    switch (a.type) {
                            case ValueType::NUMBER: {
                                    return Value(a.numberValue != b.numberValue);
                            }
                            case ValueType::BOOL: {
                                    return Value(a.boolValue != b.boolValue);
                            }
                            case ValueType::STRING: {
                                    return Value(*a.stringValue != *b.stringValue);
                            }
                            case ValueType::ARRAY: {
                                    return Value(!equalArrays(*a.arrayValue, *b.arrayValue));
                            }
                            case ValueType::NIL: {
                                    return Value(false);
                            }
                            case ValueType::FUNCTION: {
                                    return Value(a.functionValue != b.functionValue);
                            }
                            default: {
                                    return Value(true);
                            }
                    }

            } else if (op == "<" || op == ">" || op == "<=" || op == ">=") {
                    if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                            if (op == "<") {
                                    return Value(a.numberValue < b.numberValue);
                            }
                            if (op == ">") {
                                    return Value(a.numberValue > b.numberValue);
                            }
                            if (op == "<=") {
                                    return Value(a.numberValue <= b.numberValue);
                            }
                            if (op == ">=") {
                                    return Value(a.numberValue >= b.numberValue);
                            }
                    }
                    if (a.type == ValueType::STRING && b.type == ValueType::STRING) {
                            if (op == "<") {
                                    return Value(*a.stringValue < *b.stringValue);
                            }
                            if (op == ">") {
                                    return Value(*a.stringValue > *b.stringValue);
                            }
                            if (op == "<=") {
                                    return Value(*a.stringValue <= *b.stringValue);
                            }
                            if (op == ">=") {
                                    return Value(*a.stringValue >= *b.stringValue);
                            }
                    }
                    throw std::runtime_error("Оператор '" + op + "' поддерживает только числа или строки");
            }

            throw std::runtime_error("Неподдерживаемый бинарный оператор: " + op);

    }

    Value CallExpr::eval(Environment *env, std::ostream &out) {
        if (auto varExpr = dynamic_cast<VariableExpr *>(callee.get())) {
            const std::string &funcName = varExpr->name;

            auto checkArgs = [&](size_t expected) {
                if (args.size() != expected) {
                    throw std::runtime_error(
                        "Функция '" + funcName + "' ожидает ровно " + std::to_string(expected) +
                        " аргумент(а/ов), получено " + std::to_string(args.size()));
                }
            };

            if (funcName == "stacktrace") {
                    checkArgs(0);
                    std::vector<Value> frames;
                    frames.reserve(callStack.size());
                    for (const auto& name : callStack) {
                            if (name.find("<lambda at") != 0) {
                                    frames.emplace_back(std::string(name));
                            }
                    }
                    return Value(std::move(frames));
            }

            auto checkArgsRange = [&](size_t min_expected, size_t max_expected) {
                if (args.size() < min_expected || args.size() > max_expected)
                    throw std::runtime_error(
                        "Функция '" + funcName + "' ожидает от " + std::to_string(min_expected) + " до " +
                        std::to_string(max_expected) + " аргумент(а/ов), получено " + std::to_string(args.size()));
            };

            auto getArg = [&](size_t index) -> Value {
                if (index >= args.size())
                    throw std::runtime_error(
                        "Внутренняя ошибка: Попытка доступа к несуществующему аргументу " + std::to_string(index) +
                        " для '" + funcName + "'");
                return args[index]->eval(env, out);
            };

            auto expectType = [&](const Value &val, ValueType expected, size_t argIndex) {
                if (val.type != expected)
                    throw std::runtime_error(
                        "Аргумент " + std::to_string(argIndex + 1) + " функции '" + funcName + "' должен быть типа " +
                        valueTypeToString(expected) + ", получен " + valueTypeToString(val.type));
            };

            auto expectTypeOneOf = [&](const Value &val, const std::vector<ValueType> &expectedTypes, size_t argIndex) {
                bool match = false;
                std::string expectedStr;
                for (ValueType expected: expectedTypes) {
                    if (val.type == expected) {
                        match = true;
                        break;
                    }
                    if (!expectedStr.empty()) expectedStr += " или ";
                    expectedStr += valueTypeToString(expected);
                }
                if (!match)
                    throw std::runtime_error(
                        "Аргумент " + std::to_string(argIndex + 1) + " функции '" + funcName +
                        "' должен быть одного из типов (" + expectedStr + "), получен " + valueTypeToString(val.type));
            };

            if (funcName == "len") {
                checkArgs(1);
                Value arg = getArg(0);
                expectTypeOneOf(arg, {ValueType::STRING, ValueType::ARRAY}, 0);

                if (arg.type == ValueType::STRING) {
                    return Value(static_cast<double>(arg.stringValue->size()));
                } else {
                    return Value(static_cast<double>(arg.arrayValue->size()));
                }
            }
            if (funcName == "range") {
                checkArgsRange(1, 3);
                double start_val = 0.0, stop_val = 0.0, step_val = 1.0;
                if (args.size() == 1) {
                    Value stopArg = getArg(0);
                    expectType(stopArg, ValueType::NUMBER, 0);
                    stop_val = stopArg.numberValue;
                    if (!std::isfinite(stop_val))
                        throw std::runtime_error(
                            "Параметр range() должен быть конечным числом");
                } else if (args.size() >= 2) {
                    Value startArg = getArg(0);
                    expectType(startArg, ValueType::NUMBER, 0);
                    Value stopArg = getArg(1);
                    expectType(stopArg, ValueType::NUMBER, 1);
                    start_val = startArg.numberValue;
                    stop_val = stopArg.numberValue;
                    if (!std::isfinite(start_val) || !std::isfinite(stop_val))
                        throw std::runtime_error(
                            "Параметры range() должны быть конечными числами");
                    if (args.size() == 3) {
                        Value stepArg = getArg(2);
                        expectType(stepArg, ValueType::NUMBER, 2);
                        step_val = stepArg.numberValue;
                        if (!std::isfinite(step_val))
                            throw std::runtime_error(
                                "Шаг range() должен быть конечным числом");
                    }
                }
                if (step_val == 0.0) throw std::runtime_error("Шаг для функции 'range' не может быть равен 0");
                std::vector<Value> arr;
                double current = start_val;
                if (step_val > 0) {
                    while (current < stop_val) {
                        arr.emplace_back(current);
                        current += step_val;
                    }
                } else {
                    while (current > stop_val) {
                        arr.emplace_back(current);
                        current += step_val;
                    }
                }
                return Value(std::move(arr));
            }
            if (funcName == "print") {
                for (size_t i = 0; i < args.size(); ++i) {
                    Value arg = getArg(i);
                    PrintStmt::printValue(arg, out);
                }
                return {};
            }
            if (funcName == "println") {
                for (size_t i = 0; i < args.size(); ++i) {
                    Value arg = getArg(i);
                    PrintStmt::printValue(arg, out);
                }
                out << "\n";
                return {};
            }
            if (funcName == "join") {
                checkArgs(2);
                Value listVal = getArg(0);
                expectType(listVal, ValueType::ARRAY, 0);
                Value delimVal = getArg(1);
                expectType(delimVal, ValueType::STRING, 1);
                std::string delim = *delimVal.stringValue;
                std::stringstream ss;
                bool first = true;
                for (const Value &elem: *listVal.arrayValue) {
                    std::string elemStr;
                    switch (elem.type) {
                        case ValueType::STRING: elemStr = *elem.stringValue;
                            break;
                        case ValueType::NUMBER: {
                            std::stringstream num_ss;
                            PrintStmt::printValue(elem, num_ss);
                            elemStr = num_ss.str();
                            break;
                        }
                        case ValueType::BOOL: elemStr = elem.boolValue ? "true" : "false";
                            break;
                        case ValueType::NIL: elemStr = "nil";
                            break;
                        default: throw std::runtime_error(
                                "Элементы списка для 'join' должны быть конвертируемы в строку (число, строка, булево, nil)");
                    }
                    if (!first) ss << delim;
                    ss << elemStr;
                    first = false;
                }
                return Value(ss.str());
            }
            if (funcName == "abs") {
                checkArgs(1);
                Value arg = getArg(0);
                expectType(arg, ValueType::NUMBER, 0);
                return Value(std::fabs(arg.numberValue));
            }
            if (funcName == "ceil") {
                checkArgs(1);
                Value arg = getArg(0);
                expectType(arg, ValueType::NUMBER, 0);
                return Value(std::ceil(arg.numberValue));
            }
            if (funcName == "floor") {
                checkArgs(1);
                Value arg = getArg(0);
                expectType(arg, ValueType::NUMBER, 0);
                return Value(std::floor(arg.numberValue));
            }
            if (funcName == "round") {
                checkArgs(1);
                Value arg = getArg(0);
                expectType(arg, ValueType::NUMBER, 0);
                return Value(std::round(arg.numberValue));
            }
            if (funcName == "sqrt") {
                checkArgs(1);
                Value arg = getArg(0);
                expectType(arg, ValueType::NUMBER, 0);
                if (arg.numberValue < 0)
                    throw std::runtime_error(
                        "Квадратный корень из отрицательного числа невозможен для 'sqrt'");
                return Value(std::sqrt(arg.numberValue));
            }
            if (funcName == "rnd") {
                checkArgs(1);
                Value arg = getArg(0);
                expectType(arg, ValueType::NUMBER, 0);
                int n = static_cast<int>(std::floor(arg.numberValue));
                if (n <= 0) throw std::runtime_error("Аргумент функции 'rnd' должен быть положительным числом");
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, n - 1);
                return Value(static_cast<double>(dis(gen)));
            }
            if (funcName == "parse_num") {
                checkArgs(1);
                Value arg = getArg(0);
                expectType(arg, ValueType::STRING, 0);
                try {
                    size_t processed = 0;
                    std::string strToParse = *arg.stringValue;
                    double num = std::stod(strToParse, &processed);
                    if (processed != strToParse.length()) return {};
                    return Value(num);
                } catch (const std::invalid_argument &) { return {}; }
                catch (const std::out_of_range &) { return {}; }
            }
            if (funcName == "to_string") {
                checkArgs(1);
                Value arg = getArg(0);
                std::stringstream ss;
                PrintStmt::printValue(arg, ss);
                return Value(ss.str());
            }
            if (funcName == "lower") {
                checkArgs(1);
                Value arg = getArg(0);
                expectType(arg, ValueType::STRING, 0);
                std::string s = *arg.stringValue;
                std::ranges::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
                return Value(s);
            }
            if (funcName == "upper") {
                checkArgs(1);
                Value arg = getArg(0);
                expectType(arg, ValueType::STRING, 0);
                std::string s = *arg.stringValue;
                std::ranges::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
                return Value(s);
            }
            if (funcName == "split") {
                checkArgsRange(1, 2);
                Value argStr = getArg(0);
                expectType(argStr, ValueType::STRING, 0);
                Value argDelim = (args.size() == 2) ? getArg(1) : Value(" ");
                expectType(argDelim, ValueType::STRING, 1);
                std::string s = *argStr.stringValue;
                std::string delim = *argDelim.stringValue;
                std::vector<Value> parts;
                if (delim.empty()) {
                    parts.reserve(s.length());
                    for (char c: s) parts.emplace_back(std::string(1, c));
                } else {
                    size_t start = 0;
                    size_t pos = 0;
                    while ((pos = s.find(delim, start)) != std::string::npos) {
                        parts.emplace_back(s.substr(start, pos - start));
                        start = pos + delim.length();
                    }
                    parts.emplace_back(s.substr(start));
                }
                return Value(std::move(parts));
            }
            if (funcName == "replace") {
                checkArgs(3);
                Value argStr = getArg(0);
                expectType(argStr, ValueType::STRING, 0);
                Value argOld = getArg(1);
                expectType(argOld, ValueType::STRING, 1);
                Value argNew = getArg(2);
                expectType(argNew, ValueType::STRING, 2);
                std::string s = *argStr.stringValue;
                std::string oldSub = *argOld.stringValue;
                std::string newSub = *argNew.stringValue;
                if (oldSub.empty()) {
                    std::string result;
                    result.reserve(s.length() * (1 + newSub.length()) + newSub.length());
                    result += newSub;
                    for (char c: s) {
                        result += c;
                        result += newSub;
                    }
                    return Value(result);
                }
                size_t pos = 0;
                while ((pos = s.find(oldSub, pos)) != std::string::npos) {
                    s.replace(pos, oldSub.length(), newSub);
                    pos += newSub.length();
                }
                return Value(s);
            }
            if (funcName == "push") {
                if (args.size() < 2)
                    throw std::runtime_error(
                        "Функция 'push' ожидает минимум 2 аргумента (массив, элемент1, ...)");
                Value listVal = getArg(0);
                expectType(listVal, ValueType::ARRAY, 0);
                for (size_t i = 1; i < args.size(); ++i) listVal.arrayValue->push_back(getArg(i));
                return listVal;
            }
            if (funcName == "pop") {
                checkArgs(1);
                Value listVal = getArg(0);
                expectType(listVal, ValueType::ARRAY, 0);
                if (listVal.arrayValue->empty())
                    throw std::runtime_error(
                        "Невозможно выполнить 'pop' на пустом массиве");
                Value popped = listVal.arrayValue->back();
                listVal.arrayValue->pop_back();
                return popped;
            }
            if (funcName == "insert") {
                checkArgs(3);
                Value listVal = getArg(0);
                expectType(listVal, ValueType::ARRAY, 0);
                Value indexVal = getArg(1);
                expectType(indexVal, ValueType::NUMBER, 1);
                Value element = getArg(2);
                int n = static_cast<int>(listVal.arrayValue->size());
                int idx = static_cast<int>(std::floor(indexVal.numberValue));
                if (idx < 0) idx = std::max(0, n + idx);
                idx = std::min(n, idx);
                listVal.arrayValue->insert(listVal.arrayValue->begin() + idx, element);
                return listVal;
            }
            if (funcName == "remove") {
                checkArgs(2);
                Value listVal = getArg(0);
                expectType(listVal, ValueType::ARRAY, 0);
                Value indexVal = getArg(1);
                expectType(indexVal, ValueType::NUMBER, 1);
                int n = static_cast<int>(listVal.arrayValue->size());
                int idx_raw = static_cast<int>(std::floor(indexVal.numberValue));
                int idx = idx_raw;
                if (idx < 0) idx = n + idx;
                if (idx < 0 || idx >= n)
                    throw std::runtime_error(
                        "Индекс 'remove' [" + std::to_string(idx_raw) + "] вне допустимого диапазона [0.." +
                        std::to_string(n - 1) + "]");
                Value removed = (*listVal.arrayValue)[idx];
                listVal.arrayValue->erase(listVal.arrayValue->begin() + idx);
                return removed;
            }
            if (funcName == "sort") {
                checkArgs(1);
                Value listVal = getArg(0);
                expectType(listVal, ValueType::ARRAY, 0);
                if (listVal.arrayValue->size() <= 1) return listVal;
                auto compareValues = [&](const Value &a, const Value &b) -> bool {
                    if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER)
                        return a.numberValue < b.numberValue;
                    if (a.type == ValueType::STRING && b.type == ValueType::STRING)
                        return *a.stringValue < *b.stringValue;
                    throw std::runtime_error(
                        "Невозможно сравнить элементы типов " + valueTypeToString(a.type) + " и " +
                        valueTypeToString(b.type) + " для сортировки");
                };
                try {
                    std::ranges::sort(*listVal.arrayValue, compareValues);
                } catch (const std::runtime_error &e) {
                    throw std::runtime_error("Ошибка при сортировке массива: " + std::string(e.what()));
                }
                return listVal;
            }
			if (funcName == "read") {
				checkArgs(0);
				std::istream* inStream = Interpreter::CurrentIn ? Interpreter::CurrentIn : &std::cin;
				std::string inputLine;
				if (!std::getline(*inStream, inputLine)) {
					return Value(std::string(""));
				}
				return Value(inputLine);
			}
            if (funcName == "type") {
                checkArgs(1);
                Value arg = getArg(0);
                return Value(valueTypeToString(arg.type));
            }
        }
        Value funcVal = callee->eval(env, out);
        if (funcVal.type != ValueType::FUNCTION)
            throw std::runtime_error("Попытка вызвать не-функцию (тип: " + valueTypeToString(funcVal.type) + ")");
        auto func = std::move(funcVal.functionValue);
        if (callStack.size() >= MAX_CALL_DEPTH) throw std::runtime_error("Recursion depth exceeded");
        callStack.push_back(func->name);
        if (!func) throw std::runtime_error("Внутренняя ошибка: functionValue is null");
        if (args.size() != func->params.size())
            throw std::runtime_error(
                "Несоответствие количества аргументов для функции '" + func->name + "': ожидалось " +
                std::to_string(func->params.size()) + ", получено " + std::to_string(args.size()));
        Environment local(func->closure);
        for (size_t i = 0; i < func->params.size() && i < args.size(); ++i) {
            Value argVal = args[i]->eval(env, out);
            local.set_and_get_ref(func->params[i], argVal);
        }
        Value returnValue;
        try {
            for (const auto &stmt: func->bodyStmts) {
                if (!stmt) continue;
                stmt->execute(&local, out);
            }
        } catch (ReturnException &ret) {
            returnValue = ret.value;
        }
        callStack.pop_back();
        return returnValue;
    }

     NumberExpr::NumberExpr(double v): value(v) {}

     Value NumberExpr::eval(Environment*, std::ostream&) { return Value(value); }

     StringExpr::StringExpr(std::string v): value(std::move(v)) {}

     Value StringExpr::eval(Environment*, std::ostream&) { return Value(value); }

     BoolExpr::BoolExpr(bool v): value(v) {}

     Value BoolExpr::eval(Environment*, std::ostream&) { return Value(value); }

     Value NilExpr::eval(Environment*, std::ostream&) { return {}; }

     VariableExpr::VariableExpr(std::string n): name(std::move(n)) {}

     Value VariableExpr::eval(Environment *env, std::ostream&) { return env->get(name); }

     UnaryExpr::UnaryExpr(std::string oper, std::unique_ptr<Expr> expr): op(std::move(oper)),
                                                                               operand(std::move(expr)) {}

    Value UnaryExpr::eval(Environment *env, std::ostream &out) {
        Value v = operand->eval(env, out);
        if (op == "+") {
            if (v.type != ValueType::NUMBER) throw std::runtime_error("Унарный + поддерживает только числа");
            return v;
        } else if (op == "-") {
            if (v.type != ValueType::NUMBER) throw std::runtime_error("Унарный - поддерживает только числа");
            return Value(-v.numberValue);
        } else if (op == "not")
            return Value(!v.toBool());
        throw std::runtime_error("Неизвестный унарный оператор: " + op);
    }

     BinaryExpr::BinaryExpr(std::unique_ptr<Expr> l, std::string oper,
                                  std::unique_ptr<Expr> r): op(std::move(oper)), left(std::move(l)),
                                                            right(std::move(r)) {}

     CallExpr::CallExpr(std::unique_ptr<Expr> c, std::vector<std::unique_ptr<Expr> > a): callee(std::move(c)),
        args(std::move(a)) {}

     ArrayExpr::ArrayExpr(std::vector<std::unique_ptr<Expr> > elems): elements(std::move(elems)) {}

    Value ArrayExpr::eval(Environment *env, std::ostream &out) {
        std::vector<Value> vals;
        vals.reserve(elements.size());
        for (const auto &e: elements) vals.push_back(e->eval(env, out));
        return Value(std::move(vals));
    }

     IndexExpr::IndexExpr(std::unique_ptr<Expr> b, std::unique_ptr<Expr> i): base(std::move(b)),
        index(std::move(i)) {}

    Value IndexExpr::eval(Environment *env, std::ostream &out) {
        Value baseVal = base->eval(env, out);
        Value indexVal = index->eval(env, out);
        if (indexVal.type != ValueType::NUMBER) throw std::runtime_error("Индекс должен быть числом");
        double num = indexVal.numberValue;
        if (!std::isfinite(num)) throw std::runtime_error("Индекс должен быть конечным числом");
        int idx_raw = static_cast<int>(std::floor(num));
        int idx = idx_raw;
        if (baseVal.type == ValueType::ARRAY) {
            if (!baseVal.arrayValue) throw std::runtime_error("Внутренняя ошибка: arrayValue is null");
            int n = static_cast<int>(baseVal.arrayValue->size());
            if (idx < 0) {
				idx = n + idx;
			}
			if (idx < 0 || idx >= n) {
				return {};
			}
            return (*baseVal.arrayValue)[idx];
        }
        if (baseVal.type == ValueType::STRING) {
            if (!baseVal.stringValue) throw std::runtime_error("Внутренняя ошибка: stringValue is null");
            int len = static_cast<int>(baseVal.stringValue->size());
            if (idx < 0) {
				idx = len + idx;
			}
			if (idx < 0 || idx >= len) {
				return {};
			}
            return Value(std::string(1, (*baseVal.stringValue)[idx]));
        }
        throw std::runtime_error(
            "Оператор индексирования '[]' применён к неподдерживаемому типу: " + valueTypeToString(baseVal.type));
    }

     SliceExpr::SliceExpr(std::unique_ptr<Expr> base, std::unique_ptr<Expr> start,
                                std::unique_ptr<Expr> end): base(std::move(base)), start(std::move(start)),
                                                            end(std::move(end)) {}

    Value SliceExpr::eval(Environment *env, std::ostream &out) {
        Value baseVal = base->eval(env, out);
        auto get_slice_index = [&](const std::unique_ptr<Expr> &expr, int default_val, int size) -> int {
            if (!expr) return default_val;
            Value val = expr->eval(env, out);
            if (val.type != ValueType::NUMBER) throw std::runtime_error("Индекс среза должен быть числом");
            double num = val.numberValue;
            if (!std::isfinite(num)) throw std::runtime_error("Индекс среза должен быть конечным числом");
            int idx = static_cast<int>(std::floor(num));
            if (idx < 0) idx = size + idx;
            return std::max(0, std::min(idx, size));
        };
        if (baseVal.type == ValueType::ARRAY) {
            if (!baseVal.arrayValue) throw std::runtime_error("Внутренняя ошибка: arrayValue is null for slice");
            int n = static_cast<int>(baseVal.arrayValue->size());
            int s = get_slice_index(start, 0, n);
            int e = get_slice_index(end, n, n);
            if (s >= e) return Value(std::vector<Value>{});
            std::vector<Value> slice;
            slice.reserve(e - s);
            auto it_start = baseVal.arrayValue->begin() + s;
            auto it_end = baseVal.arrayValue->begin() + e;
            for (auto it = it_start; it != it_end; ++it) slice.push_back(*it);
            return Value(std::move(slice));
        }
        if (baseVal.type == ValueType::STRING) {
            if (!baseVal.stringValue) throw std::runtime_error("Внутренняя ошибка: stringValue is null for slice");
            int n = static_cast<int>(baseVal.stringValue->size());
            int s = get_slice_index(start, 0, n);
            int e = get_slice_index(end, n, n);
            if (s >= e) return Value("");
            return Value(baseVal.stringValue->substr(s, e - s));
        }
        throw std::runtime_error(
            "Оператор среза '[:]' применён к неподдерживаемому типу: " + valueTypeToString(baseVal.type));
    }

     FunctionExpr::FunctionExpr(std::vector<std::string> p,
                                      std::vector<std::unique_ptr<Stmt> > b,
                                      int line, int col)
        : params(std::move(p))
          , body(std::move(b))
          , def_line(line)
          , def_col(col) {}

    Value FunctionExpr::eval(Environment *env, std::ostream&) {
        auto func = std::make_shared<FunctionDef>();
        func->params = params;
        func->bodyStmts = std::move(body);
        func->closure = env->getGlobal();
        func->name = "<lambda at "
                     + std::to_string(def_line)
                     + ":"
                     + std::to_string(def_col)
                     + ">";
        return Value(func);
    }
} //Interpreter