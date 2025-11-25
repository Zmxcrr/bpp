#include "statement.h"
#include "expression.h"
#include <cmath>
#include <iostream>
#include <algorithm>

namespace Interpreter {
     Stmt::~Stmt() = default;

     void Stmt::execute(Environment*, std::ostream&) {}

     ExpressionStmt::ExpressionStmt(std::unique_ptr<Expr> e): expr(std::move(e)) {}

     void ExpressionStmt::execute(Environment *env, std::ostream &out) { expr->eval(env, out); }

     PrintStmt::PrintStmt(std::unique_ptr<Expr> e): expr(std::move(e)) {}

    void PrintStmt::printValue(const Value &v, std::ostream &out) {
        switch (v.type) {
            case ValueType::NUMBER:
                if (std::floor(v.numberValue) == v.numberValue &&
                    v.numberValue >= -9007199254740991.0 &&
                    v.numberValue <= 9007199254740991.0) {
                    out << static_cast<long long>(v.numberValue);
                } else out << v.numberValue;
                break;
            case ValueType::STRING: {
                if (!v.stringValue) {
                    out << "<null string>";
                    break;
                }
				out << *v.stringValue;
                break;
            }
			case ValueType::BOOL:
				out << (v.boolValue ? 1 : 0);
				break;
            case ValueType::NIL: out << "nil";
                break;
			case ValueType::ARRAY: {
				if (!v.arrayValue) {
					out << "<null array>";
					break;
				}

				auto printQuotedString = [&](const std::string& s) {
					out << "\"";
					for (char c : s) {
						switch (c) {
							case '\\': out << "\\\\"; break;
							case '"':  out << "\\\""; break;
							case '\n': out << "\\n";  break;
							case '\t': out << "\\t";  break;
							default:   out << c;      break;
						}
					}
					out << "\"";
				};

				out << "[";
				for (size_t i = 0; i < v.arrayValue->size(); ++i) {
					const Value& elem = (*v.arrayValue)[i];
					if (elem.type == ValueType::STRING) {
						if (!elem.stringValue) out << "\"<null string>\"";
						else printQuotedString(*elem.stringValue);
					} else {
						printValue(elem, out);
					}
					if (i + 1 < v.arrayValue->size()) out << ", ";
				}
				out << "]";
				break;
			}
            case ValueType::FUNCTION:
                if (!v.functionValue) out << "<null function>";
                else out << "<function>";
                break;
            default: out << "<unknown_value_type:" << static_cast<int>(v.type) << ">";
                break;
        }
    }

    void PrintStmt::execute(Environment *env, std::ostream &out) {
        Value v = expr->eval(env, out);
        printValue(v, out);
    }

     PrintlnStmt::PrintlnStmt(std::unique_ptr<Expr> e): expr(std::move(e)) {}

    void PrintlnStmt::execute(Environment *env, std::ostream &out) {
        if (expr) {
            Value v = expr->eval(env, out);
            PrintStmt::printValue(v, out);
        }
        out << "\n";
    }

     ReturnStmt::ReturnStmt(std::unique_ptr<Expr> e): expr(std::move(e)) {}

    void ReturnStmt::execute(Environment *env, std::ostream &out) {
        Value v;
        if (expr) v = expr->eval(env, out);
        throw ReturnException(v);
    }

     AssignStmt::AssignStmt(std::string n, std::unique_ptr<Expr> e): name(std::move(n)), expr(std::move(e)) {}

    void AssignStmt::execute(Environment *env, std::ostream &out) {
        Value v = expr->eval(env, out);
        env->set(name, v);
    }

     CompoundAssignStmt::CompoundAssignStmt(std::string n, std::string o,
                                                  std::unique_ptr<Expr> e): name(std::move(n)),
                                                                            op(std::move(o)), expr(std::move(e)) {}

    void CompoundAssignStmt::execute(Environment *env, std::ostream &out) {
        Value oldValue = env->get(name);
        Value rightValue = expr->eval(env, out);
        Value newValue = applyBinaryOp(oldValue, rightValue, op);
        env->set(name, newValue);
    }

     AssignIndexStmt::AssignIndexStmt(std::unique_ptr<Expr> b, std::unique_ptr<Expr> i,
                                            std::unique_ptr<Expr> v): base(std::move(b)),
                                                                      index(std::move(i)), value(std::move(v)) {}

    void AssignIndexStmt::execute(Environment *env, std::ostream &out) {
        Value bval = base->eval(env, out);
        Value idxVal = index->eval(env, out);
        if (idxVal.type != ValueType::NUMBER) throw std::runtime_error("Индекс должен быть числом");
        int raw = static_cast<int>(std::floor(idxVal.numberValue));
        if (bval.type == ValueType::ARRAY) {
            auto &arr = *bval.arrayValue;
            int n = static_cast<int>(arr.size());
            int idx = raw < 0 ? n + raw : raw;
            if (idx < 0 || idx >= n) throw std::runtime_error("Индекс вне диапазона при присваивании");
            arr[idx] = value->eval(env, out);
        } else if (bval.type == ValueType::STRING) {
            auto &s = *bval.stringValue;
            int n = static_cast<int>(s.size());
            int idx = raw < 0 ? n + raw : raw;
            if (idx < 0 || idx >= n) throw std::runtime_error("Индекс вне диапазона при присваивании");
            Value v = value->eval(env, out);
            if (v.type != ValueType::STRING || v.stringValue->size() != 1)
                throw std::runtime_error(
                    "Для присваивания символа ожидается строка длины 1");
            s[idx] = (*v.stringValue)[0];
        } else throw std::runtime_error("Нельзя присвоить по индексу тип: " + valueTypeToString(bval.type));
    }

     AssignSliceStmt::AssignSliceStmt(std::unique_ptr<Expr> b, std::unique_ptr<Expr> s, std::unique_ptr<Expr> e,
                                            std::unique_ptr<Expr> v): base(std::move(b)), start(std::move(s)),
                                                                      end(std::move(e)),
                                                                      value(std::move(v)) {
    }

     void AssignSliceStmt::execute(Environment *env, std::ostream &out) {
        Value bval = base->eval(env, out);
        auto evalIdx = [&](Expr *ex, int def, int sz)-> int {
            if (!ex) return def;
            Value vv = ex->eval(env, out);
            if (vv.type != ValueType::NUMBER) throw std::runtime_error("Срез: индекс должен быть числом");
            int x = static_cast<int>(std::floor(vv.numberValue));
            return x < 0 ? sz + x : x;
        };
        if (bval.type == ValueType::ARRAY) {
            auto &arr = *bval.arrayValue;
            int n = static_cast<int>(arr.size());
            int s = std::clamp(evalIdx(start.get(), 0, n), 0, n);
            int e = std::clamp(evalIdx(end.get(), n, n), 0, n);
            Value v = value->eval(env, out);
            if (v.type != ValueType::ARRAY) throw std::runtime_error("Для присваивания срезу требуется массив");
            auto &src = *v.arrayValue;
            arr.erase(arr.begin() + s, arr.begin() + e);
            arr.insert(arr.begin() + s, src.begin(), src.end());
        } else if (bval.type == ValueType::STRING) {
            auto &sstr = *bval.stringValue;
            int n = static_cast<int>(sstr.size());
            int s = std::clamp(evalIdx(start.get(), 0, n), 0, n);
            int e = std::clamp(evalIdx(end.get(), n, n), 0, n);
            Value v = value->eval(env, out);
            if (v.type != ValueType::STRING) throw std::runtime_error("Для присваивания срезу требуется строка");
            sstr.replace(s, e - s, *v.stringValue);
        } else throw std::runtime_error("Нельзя присвоить по срезу тип: " + valueTypeToString(bval.type));
    }

     IfStmt::IfStmt(std::unique_ptr<Expr> cond, std::vector<std::unique_ptr<Stmt> > thenB,
                          std::vector<std::pair<std::unique_ptr<Expr>, std::vector<std::unique_ptr<Stmt> > > > elseIfB,
                          std::vector<std::unique_ptr<Stmt> > elseB): condition(std::move(cond)),
                                                                      thenBranch(std::move(thenB)),
                                                                      elseIfBranches(std::move(elseIfB)),
                                                                      elseBranch(std::move(elseB)) {}

    void IfStmt::execute(Environment *env, std::ostream &out) {
        Value condVal = condition->eval(env, out);
        if (condVal.toBool()) {
            Environment blockEnv(env);
            for (const auto &stmt: thenBranch) {
                if (!stmt) continue;
                stmt->execute(&blockEnv, out);
            }
        } else {
            bool executedElseIf = false;
            for (auto &elseIfPair: elseIfBranches) {
                if (!elseIfPair.first) continue;
                Value elseIfCondVal = elseIfPair.first->eval(env, out);
                if (elseIfCondVal.toBool()) {
                    Environment blockEnv(env);
                    for (const auto &stmt: elseIfPair.second) {
                        if (!stmt) continue;
                        stmt->execute(&blockEnv, out);
                    }
                    executedElseIf = true;
                    break;
                }
            }
            if (!executedElseIf) {
                Environment blockEnv(env);
                for (const auto &stmt: elseBranch) {
                    if (!stmt) continue;
                    stmt->execute(&blockEnv, out);
                }
            }
        }
    }

     ForStmt::ForStmt(std::string v, std::unique_ptr<Expr> iter,
                            std::vector<std::unique_ptr<Stmt> > b): var(std::move(v)),
                                                                    iterable(std::move(iter)), body(std::move(b)) {}

    void ForStmt::execute(Environment *env, std::ostream &out) {
        Value iterVal = iterable->eval(env, out);
        if (iterVal.type == ValueType::ARRAY) {
            if (!iterVal.arrayValue) throw std::runtime_error("Внутренняя ошибка: arrayValue null в цикле for");
            const auto &arrayRef = *iterVal.arrayValue;

            for (const Value &elem: arrayRef) {
                Environment loopEnv(env);
                loopEnv.set_and_get_ref(var, elem);
                try {
                    for (const auto &stmt: body) {
                        if (!stmt) continue;
                        stmt->execute(&loopEnv, out);
                    }
                } catch (BreakException &) { break; }
                catch (ContinueException &) { continue; }
            }
        } else if (iterVal.type == ValueType::STRING) {
            if (!iterVal.stringValue) throw std::runtime_error("Внутренняя ошибка: stringValue null в цикле for");
            const std::string &stringRef = *iterVal.stringValue;
            for (char c: stringRef) {
                Environment loopEnv(env);
                loopEnv.set_and_get_ref(var, Value(std::string(1, c)));
                try {
                    for (const auto &stmt: body) {
                        if (!stmt) continue;
                        stmt->execute(&loopEnv, out);
                    }
                } catch (BreakException &) { break; }
                catch (ContinueException &) { continue; }
            }
        } else {
            throw std::runtime_error(
                "Итерабельное значение в цикле for должно быть массивом или строкой, получено: "
                + valueTypeToString(iterVal.type)
            );
        }
    }

     WhileStmt::WhileStmt(std::unique_ptr<Expr> cond,
                                std::vector<std::unique_ptr<Stmt> > b): condition(std::move(cond)), body(std::move(b)) {}

    void WhileStmt::execute(Environment *env, std::ostream &out) {
        while (true) {
            if (!condition) throw std::runtime_error("Внутренняя ошибка: null condition в while");
            Value condVal = condition->eval(env, out);
            if (!condVal.toBool()) break;
            try {
                Environment blockEnv(env);
                for (const auto &stmt: body) {
                    if (!stmt) continue;
                    stmt->execute(&blockEnv, out);
                }
            } catch (BreakException &) {
                break;
            } catch (ContinueException &) {
                continue;
            }
        }
    }

     FunctionStmt::FunctionStmt(std::string n, std::vector<std::string> p,
                                      std::vector<std::unique_ptr<Stmt> > b): name(std::move(n)), params(std::move(p)),
                                                                              body(std::move(b)) {}

    void FunctionStmt::execute(Environment* env, std::ostream&) {
        auto func = std::make_shared<FunctionDef>();
        func->params = params;
        func->bodyStmts = std::move(body);
        func->closure = env->getGlobal();
        func->name = name;
        Value funcValue(func);
        env->set(name, funcValue);
    }

     void BreakStmt::execute(Environment*, std::ostream&) { throw BreakException(); }

     void ContinueStmt::execute(Environment*, std::ostream&) {
        throw ContinueException();
    }

    Value applyBinaryOp(const Value &a, const Value &b, const std::string &op) {
        if (op == "+") {
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) return Value(a.numberValue + b.numberValue);
            if (a.type == ValueType::STRING && b.type == ValueType::STRING)
                return Value(
                    *a.stringValue + *b.stringValue);
            if (a.type == ValueType::ARRAY && b.type == ValueType::ARRAY) {
                std::vector<Value> result = *a.arrayValue;
                result.insert(result.end(), b.arrayValue->begin(), b.arrayValue->end());
                return Value(std::move(result));
            }
            if (a.type == ValueType::STRING) {
                std::ostringstream ss;
                PrintStmt::printValue(b, ss);
                return Value(*a.stringValue + ss.str());
            }
            if (b.type == ValueType::STRING) {
                std::ostringstream ss;
                PrintStmt::printValue(a, ss);
                return Value(ss.str() + *b.stringValue);
            }
            throw std::runtime_error("Оператор '+' поддерживает только числа, строки или списки одного типа");
        } else if (op == "-") {
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) return Value(a.numberValue - b.numberValue);
            throw std::runtime_error("Оператор '-' поддерживает только числа");
        } else if (op == "*") {
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) return Value(a.numberValue * b.numberValue);
            if (a.type == ValueType::STRING && b.type == ValueType::NUMBER) {
                int times = static_cast<int>(std::floor(b.numberValue));
                if (times < 0) times = 0;
                std::string res;
                if (times > 0) res.reserve(a.stringValue->size() * static_cast<size_t>(times));
                for (int i = 0; i < times; ++i) res += *a.stringValue;
                return Value(res);
            }
            if (a.type == ValueType::STRING && b.type == ValueType::BOOL) {
                int times = b.boolValue ? 1 : 0;
                std::string res;
                for (int i = 0; i < times; ++i) res += *a.stringValue;
                return Value(res);
            }
            if (a.type == ValueType::ARRAY && b.type == ValueType::NUMBER) {
                int times = static_cast<int>(std::floor(b.numberValue));
                if (times < 0) times = 0;
                std::vector<Value> res;
                if (times > 0) res.reserve(a.arrayValue->size() * static_cast<size_t>(times));
                for (int i = 0; i < times; ++i) res.insert(res.end(), a.arrayValue->begin(), a.arrayValue->end());
                return Value(std::move(res));
            }
            throw std::runtime_error(
                "Оператор '*' не поддерживается для данных типов (" + valueTypeToString(a.type) + ", " +
                valueTypeToString(b.type) + ")");
        } else if (op == "/") {
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                if (b.numberValue == 0) throw std::runtime_error("Деление на ноль для '/'");
                return Value(a.numberValue / b.numberValue);
            }
            throw std::runtime_error("Оператор '/' поддерживает только числа");
        } else if (op == "%") {
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER) {
                if (b.numberValue == 0) throw std::runtime_error("Деление на ноль в операции '%'");
                return Value(std::fmod(a.numberValue, b.numberValue));
            }
            throw std::runtime_error("Оператор '%' поддерживает только числа");
        } else if (op == "^") {
            if (a.type == ValueType::NUMBER && b.type == ValueType::NUMBER)
                return Value(
                    std::pow(a.numberValue, b.numberValue));
            throw std::runtime_error("Оператор '^' поддерживает только числа");
        }
        throw std::runtime_error("Неподдерживаемый составной оператор присваивания: " + op + "=");
    }

} //Interpreter