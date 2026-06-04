#pragma once
#include "Token.h"
#include <memory>

// 순수 가상 인터페이스를 표현하기 위한 키워드 (struct 의 별칭)
#ifndef interface
#define interface struct
#endif

// Forward declarations
class LiteralExpr;
class UnaryExpr;
class BinaryExpr;
class GroupingExpr;
class VariableExpr;
class AssignExpr;

// ── Visitor interface ──────────────────────────────────────────────────────────
interface ExprVisitor{
    virtual ValuableValue visitLiteralExpr(LiteralExpr & expr) = 0;
    virtual ValuableValue visitUnaryExpr(UnaryExpr& expr) = 0;
    virtual ValuableValue visitBinaryExpr(BinaryExpr& expr) = 0;
    virtual ValuableValue visitGroupingExpr(GroupingExpr& expr) = 0;
    virtual ValuableValue visitVariableExpr(VariableExpr& expr) = 0;
    virtual ValuableValue visitAssignExpr(AssignExpr& expr) = 0;
    virtual ~ExprVisitor() = default;
};

// ── Base Expression Node ───────────────────────────────────────────────────────
interface Expr{
    virtual ValuableValue accept(ExprVisitor & visitor) = 0;
    virtual ~Expr() = default;
};
using ExprPtr = std::unique_ptr<Expr>;

// ── Concrete Expression Nodes ──────────────────────────────────────────────────

// 숫자, 문자열, bool, nil 리터럴
class LiteralExpr : public Expr {
public:
    explicit LiteralExpr(ValuableValue v) : value(std::move(v)) {}
    ValuableValue accept(ExprVisitor& v) override { return v.visitLiteralExpr(*this); }

    const ValuableValue& getValue() const { return value; }

private:
    ValuableValue value;
};

// 단항 연산자: -expr
class UnaryExpr : public Expr {
public:
    UnaryExpr(Token op, ExprPtr right) : op(std::move(op)), right(std::move(right)) {}
    ValuableValue accept(ExprVisitor& v) override { return v.visitUnaryExpr(*this); }

    const Token& getOp()    const { return op; }
    const ExprPtr& getRight() const { return right; }

private:
    Token   op;
    ExprPtr right;
};

// 이항 연산자: left op right
class BinaryExpr : public Expr {
public:
    BinaryExpr(ExprPtr l, Token op, ExprPtr r)
        : left(std::move(l)), op(std::move(op)), right(std::move(r)) {
    }
    ValuableValue accept(ExprVisitor& v) override { return v.visitBinaryExpr(*this); }

    const ExprPtr& getLeft()  const { return left; }
    const Token& getOp()    const { return op; }
    const ExprPtr& getRight() const { return right; }

private:
    ExprPtr left;
    Token   op;
    ExprPtr right;
};

// 괄호 그룹: ( expr )
class GroupingExpr : public Expr {
public:
    explicit GroupingExpr(ExprPtr expr) : expression(std::move(expr)) {}
    ValuableValue accept(ExprVisitor& v) override { return v.visitGroupingExpr(*this); }

    const ExprPtr& getExpression() const { return expression; }

private:
    ExprPtr expression;
};

// 변수 참조: identifier
class VariableExpr : public Expr {
public:
    explicit VariableExpr(Token name) : name(std::move(name)) {}
    ValuableValue accept(ExprVisitor& v) override { return v.visitVariableExpr(*this); }

    const Token& getName() const { return name; }

private:
    Token name;
};

// 대입: identifier = expr
class AssignExpr : public Expr {
public:
    AssignExpr(Token name, ExprPtr val) : name(std::move(name)), value(std::move(val)) {}
    ValuableValue accept(ExprVisitor& v) override { return v.visitAssignExpr(*this); }

    const Token& getName()  const { return name; }
    const ExprPtr& getValue() const { return value; }

private:
    Token   name;
    ExprPtr value;
};
