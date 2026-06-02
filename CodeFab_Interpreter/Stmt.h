#pragma once
#include "Expr.h"
#include <vector>
#include <memory>

// Forward declarations
class PrintStmt;
class ExprStmt;
class VarStmt;
class BlockStmt;
class IfStmt;
class ForStmt;

// ── Visitor interface ──────────────────────────────────────────────────────────
interface StmtVisitor{
    virtual void visitPrintStmt(PrintStmt & stmt) = 0;
    virtual void visitExprStmt(ExprStmt& stmt) = 0;
    virtual void visitVarStmt(VarStmt& stmt) = 0;
    virtual void visitBlockStmt(BlockStmt& stmt) = 0;
    virtual void visitIfStmt(IfStmt& stmt) = 0;
    virtual void visitForStmt(ForStmt& stmt) = 0;
    virtual ~StmtVisitor() = default;
};

// ── Base Statement Node ────────────────────────────────────────────────────────
interface Stmt{
    virtual void accept(StmtVisitor & visitor) = 0;
    virtual ~Stmt() = default;
};
using StmtPtr = std::unique_ptr<Stmt>;

// ── Concrete Statement Nodes ───────────────────────────────────────────────────

// print expr ;
class PrintStmt : public Stmt {
public:
    explicit PrintStmt(ExprPtr expr) : expression(std::move(expr)) {}
    void accept(StmtVisitor& v) override { v.visitPrintStmt(*this); }

    const ExprPtr& getExpression() const { return expression; }

private:
    ExprPtr expression;
};

// expr ;
class ExprStmt : public Stmt {
public:
    explicit ExprStmt(ExprPtr expr) : expression(std::move(expr)) {}
    void accept(StmtVisitor& v) override { v.visitExprStmt(*this); }

    const ExprPtr& getExpression() const { return expression; }

private:
    ExprPtr expression;
};

// var name = initializer ;
class VarStmt : public Stmt {
public:
    VarStmt(Token name, ExprPtr init) : name(std::move(name)), initializer(std::move(init)) {}
    void accept(StmtVisitor& v) override { v.visitVarStmt(*this); }

    const Token& getName()        const { return name; }
    const ExprPtr& getInitializer() const { return initializer; } // nullptr → nil

private:
    Token   name;
    ExprPtr initializer;
};

// { statements }
class BlockStmt : public Stmt {
public:
    explicit BlockStmt(std::vector<StmtPtr> stmts) : statements(std::move(stmts)) {}
    void accept(StmtVisitor& v) override { v.visitBlockStmt(*this); }

    const std::vector<StmtPtr>& getStatements() const { return statements; }

private:
    std::vector<StmtPtr> statements;
};

// if ( condition ) thenBranch [else elseBranch]
class IfStmt : public Stmt {
public:
    IfStmt(ExprPtr cond, StmtPtr then_, StmtPtr else_)
        : condition(std::move(cond)), thenBranch(std::move(then_)), elseBranch(std::move(else_)) {
    }
    void accept(StmtVisitor& v) override { v.visitIfStmt(*this); }

    const ExprPtr& getCondition()  const { return condition; }
    const StmtPtr& getThenBranch() const { return thenBranch; }
    const StmtPtr& getElseBranch() const { return elseBranch; } // nullptr if no else

private:
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;
};

// for ( init ; condition ; increment ) body
class ForStmt : public Stmt {
public:
    ForStmt(StmtPtr init, ExprPtr cond, ExprPtr incr, StmtPtr body)
        : initializer(std::move(init)), condition(std::move(cond))
        , increment(std::move(incr)), body(std::move(body)) {
    }
    void accept(StmtVisitor& v) override { v.visitForStmt(*this); }

    const StmtPtr& getInitializer() const { return initializer; } // nullptr if omitted
    const ExprPtr& getCondition()   const { return condition; }   // nullptr → run forever
    const ExprPtr& getIncrement()   const { return increment; }   // nullptr if omitted
    const StmtPtr& getBody()        const { return body; }

private:
    StmtPtr initializer;
    ExprPtr condition;
    ExprPtr increment;
    StmtPtr body;
};
