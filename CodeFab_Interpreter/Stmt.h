#pragma once
#include "Expr.h"
#include <vector>
#include <memory>

class PrintStmt;
class ExprStmt;
class VarStmt;
class BlockStmt;
class IfStmt;
class ForStmt;

interface StmtVisitor{
    virtual void visitPrintStmt(PrintStmt & stmt) = 0;
    virtual void visitExprStmt(ExprStmt& stmt) = 0;
    virtual void visitVarStmt(VarStmt& stmt) = 0;
    virtual void visitBlockStmt(BlockStmt& stmt) = 0;
    virtual void visitIfStmt(IfStmt& stmt) = 0;
    virtual void visitForStmt(ForStmt& stmt) = 0;
    virtual ~StmtVisitor() = default;
};

interface Stmt{
    virtual void accept(StmtVisitor & visitor) = 0;
    virtual ~Stmt() = default;
};
using StmtPtr = std::unique_ptr<Stmt>;


class PrintStmt : public Stmt {
public:
    explicit PrintStmt(ExprPtr expr) : expression(std::move(expr)) {}
    void accept(StmtVisitor& v) override { v.visitPrintStmt(*this); }

    ExprPtr& getExpression() { return expression; }

private:
    ExprPtr expression;
};


class ExprStmt : public Stmt {
public:
    explicit ExprStmt(ExprPtr expr) : expression(std::move(expr)) {}
    void accept(StmtVisitor& v) override { v.visitExprStmt(*this); }

    ExprPtr& getExpression() { return expression; }

private:
    ExprPtr expression;
};

class VarStmt : public Stmt {
public:
    VarStmt(Token name, ExprPtr init) : name(std::move(name)), initializer(std::move(init)) {}
    void accept(StmtVisitor& v) override { v.visitVarStmt(*this); }

    const Token& getName()        const { return name; }
    ExprPtr& getInitializer() { return initializer; } // nullptr → nil

private:
    Token   name;
    ExprPtr initializer;
};

class BlockStmt : public Stmt {
public:
    explicit BlockStmt(std::vector<StmtPtr> stmts) : statements(std::move(stmts)) {}
    void accept(StmtVisitor& v) override { v.visitBlockStmt(*this); }

    std::vector<StmtPtr>& getStatements() { return statements; }

private:
    std::vector<StmtPtr> statements;
};

class IfStmt : public Stmt {
public:
    IfStmt(ExprPtr cond, StmtPtr then_, StmtPtr else_)
        : condition(std::move(cond)), thenBranch(std::move(then_)), elseBranch(std::move(else_)) {
    }
    void accept(StmtVisitor& v) override { v.visitIfStmt(*this); }

    ExprPtr& getCondition()  { return condition; }
    StmtPtr& getThenBranch() { return thenBranch; }
    StmtPtr& getElseBranch() { return elseBranch; } // nullptr if no else

private:
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;
};

class ForStmt : public Stmt {
public:
    ForStmt(StmtPtr init, ExprPtr cond, ExprPtr incr, StmtPtr body)
        : initializer(std::move(init)), condition(std::move(cond))
        , increment(std::move(incr)), body(std::move(body)) {
    }
    void accept(StmtVisitor& v) override { v.visitForStmt(*this); }

    StmtPtr& getInitializer() { return initializer; } // nullptr if omitted
    ExprPtr& getCondition()   { return condition; }   // nullptr → run forever
    ExprPtr& getIncrement()   { return increment; }   // nullptr if omitted
    StmtPtr& getBody()        { return body; }

private:
    StmtPtr initializer;
    ExprPtr condition;
    ExprPtr increment;
    StmtPtr body;
};
