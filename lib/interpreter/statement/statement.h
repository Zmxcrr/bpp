#pragma once

#include "../value/value.h"
#include "../environment/environment.h"

namespace Interpreter {
    struct Expr;

    struct Stmt {
        virtual ~Stmt();

        virtual void execute(Environment*, std::ostream&);
    };

    struct ExpressionStmt : public Stmt {
        std::unique_ptr<Expr> expr;

        explicit ExpressionStmt(std::unique_ptr<Expr>);

        void execute(Environment*, std::ostream&) override;
    };

    struct PrintStmt : public Stmt {
        std::unique_ptr<Expr> expr;

        explicit PrintStmt(std::unique_ptr<Expr>);

        static void printValue(const Value&, std::ostream&);

        void execute(Environment*, std::ostream&) override;
    };

    struct PrintlnStmt : public Stmt {
        std::unique_ptr<Expr> expr;

        explicit PrintlnStmt(std::unique_ptr<Expr>);

        void execute(Environment*, std::ostream&) override;
    };

    struct ReturnStmt : public Stmt {
        std::unique_ptr<Expr> expr;

        explicit ReturnStmt(std::unique_ptr<Expr> = nullptr);

        void execute(Environment*, std::ostream&) override;
    };

    struct AssignStmt : public Stmt {
        std::string name;
        std::unique_ptr<Expr> expr;

        AssignStmt(std::string, std::unique_ptr<Expr>);

        void execute(Environment*, std::ostream&) override;
    };

    Value applyBinaryOp(const Value&, const Value&, const std::string&);

    struct CompoundAssignStmt : public Stmt {
        std::string name;
        std::string op;
        std::unique_ptr<Expr> expr;

        CompoundAssignStmt(std::string, std::string, std::unique_ptr<Expr>);

        void execute(Environment*, std::ostream&) override;
    };

    struct AssignIndexStmt : public Stmt {
        std::unique_ptr<Expr> base, index, value;

        AssignIndexStmt(std::unique_ptr<Expr> b, std::unique_ptr<Expr> i, std::unique_ptr<Expr> v);

        void execute(Environment*, std::ostream&) override;
    };

    struct AssignSliceStmt : public Stmt {
        std::unique_ptr<Expr> base, start, end, value;

        AssignSliceStmt(std::unique_ptr<Expr> b, std::unique_ptr<Expr> s, std::unique_ptr<Expr> e,
                        std::unique_ptr<Expr> v);

        void execute(Environment*, std::ostream&) override;
    };

    struct IfStmt : public Stmt {
        std::unique_ptr<Expr> condition;
        std::vector<std::unique_ptr<Stmt> > thenBranch;
        std::vector<std::pair<std::unique_ptr<Expr>, std::vector<std::unique_ptr<Stmt> > > > elseIfBranches;
        std::vector<std::unique_ptr<Stmt> > elseBranch;

        IfStmt(std::unique_ptr<Expr> cond,
               std::vector<std::unique_ptr<Stmt> > thenB,
               std::vector<std::pair<std::unique_ptr<Expr>,
                   std::vector<std::unique_ptr<Stmt> > > > elseIfB,
               std::vector<std::unique_ptr<Stmt> > elseB);

        void execute(Environment*, std::ostream&) override;
    };

    struct ForStmt : public Stmt {
        std::string var;
        std::unique_ptr<Expr> iterable;
        std::vector<std::unique_ptr<Stmt> > body;

        ForStmt(std::string v, std::unique_ptr<Expr> iter, std::vector<std::unique_ptr<Stmt> > b);

        void execute(Environment*, std::ostream&) override;
    };

    struct WhileStmt : public Stmt {
        std::unique_ptr<Expr> condition;
        std::vector<std::unique_ptr<Stmt> > body;

        WhileStmt(std::unique_ptr<Expr> cond, std::vector<std::unique_ptr<Stmt> > b);

        void execute(Environment *env, std::ostream &out) override;
    };

    struct FunctionStmt : public Stmt {
        std::string name;
        std::vector<std::string> params;
        std::vector<std::unique_ptr<Stmt> > body;

        FunctionStmt(std::string n, std::vector<std::string> p,
                     std::vector<std::unique_ptr<Stmt> > b);

        void execute(Environment*, std::ostream&) override;
    };

    struct BreakStmt : public Stmt {
        void execute(Environment*, std::ostream&) override;
    };

    struct ContinueStmt : public Stmt {
        void execute(Environment*, std::ostream&) override;
    };
} //Interpreter
