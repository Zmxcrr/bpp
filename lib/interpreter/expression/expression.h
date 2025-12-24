#pragma once

#include "value.h"
#include "environment.h"
#include "interpreter.h"
#include "functiondef.h"
#include "exception.h"
#include "statement.h"

namespace Interpreter {
    struct Expr {
        virtual ~Expr();

        virtual Value eval(Environment* env, std::ostream& out);
    };

    struct NumberExpr : public Expr {
        double value;

        explicit NumberExpr(double v);

        Value eval(Environment*, std::ostream&) override;
    };

    struct StringExpr : public Expr {
        std::string value;

        explicit StringExpr(std::string v);

        Value eval(Environment*, std::ostream&) override;
    };

    struct BoolExpr : public Expr {
        bool value;

        explicit BoolExpr(bool v);

        Value eval(Environment*, std::ostream&) override;
    };

    struct NilExpr : public Expr {
        Value eval(Environment*, std::ostream&) override;
    };

    struct VariableExpr : public Expr {
        std::string name;

        explicit VariableExpr(std::string n);

        Value eval(Environment*, std::ostream&) override;
    };

    struct UnaryExpr : public Expr {
        std::string op;
        std::unique_ptr<Expr> operand;

        UnaryExpr(std::string oper, std::unique_ptr<Expr> expr);

        Value eval(Environment*, std::ostream&) override;
    };

    struct BinaryExpr : public Expr {
        std::string op;
        std::unique_ptr<Expr> left, right;

        BinaryExpr(std::unique_ptr<Expr> l, std::string oper, std::unique_ptr<Expr> r);

        Value eval(Environment*, std::ostream&) override;
    };

    struct CallExpr : public Expr {
        std::unique_ptr<Expr> callee;
        std::vector<std::unique_ptr<Expr> > args;

        CallExpr(std::unique_ptr<Expr> c, std::vector<std::unique_ptr<Expr> > a);

        Value eval(Environment*, std::ostream&) override;
    };

    struct ArrayExpr : public Expr {
        std::vector<std::unique_ptr<Expr> > elements;

        explicit ArrayExpr(std::vector<std::unique_ptr<Expr> > elems);

        Value eval(Environment*, std::ostream&) override;
    };

    struct IndexExpr : public Expr {
        std::unique_ptr<Expr> base;
        std::unique_ptr<Expr> index;

        IndexExpr(std::unique_ptr<Expr> b, std::unique_ptr<Expr> i);

        Value eval(Environment*, std::ostream&) override;
    };

    struct SliceExpr : public Expr {
        std::unique_ptr<Expr> base;
        std::unique_ptr<Expr> start;
        std::unique_ptr<Expr> end;

        SliceExpr(std::unique_ptr<Expr> base, std::unique_ptr<Expr> start,
                  std::unique_ptr<Expr> end);

        Value eval(Environment*, std::ostream&) override;
    };

    struct FunctionExpr : public Expr {
        std::vector<std::string> params;
        std::vector<std::unique_ptr<Stmt> > body;
        int def_line;
        int def_col;

        FunctionExpr(std::vector<std::string> p, std::vector<std::unique_ptr<Stmt> > b, int line, int col);

        Value eval(Environment *env, std::ostream &) override;
    };
} //Interpreter
