#pragma once
#include "Token.h"
#include <memory>
#define interface struct

class LiteralExpr;
class UnaryExpr;
class BinaryExpr;
class GroupingExpr;
class VariableExpr;
class AssignExpr;

interface ExprVisitor{
    virtual ValuableValue visitLiteralExpr(LiteralExpr & expr) = 0;
    virtual ValuableValue visitUnaryExpr(UnaryExpr& expr) = 0;
    virtual ValuableValue visitBinaryExpr(BinaryExpr& expr) = 0;
    virtual ValuableValue visitGroupingExpr(GroupingExpr& expr) = 0;
    virtual ValuableValue visitVariableExpr(VariableExpr& expr) = 0;
    virtual ValuableValue visitAssignExpr(AssignExpr& expr) = 0;
    virtual ~ExprVisitor() = default;
};

interface Expr{
    virtual ValuableValue accept(ExprVisitor & visitor) = 0;
    virtual ~Expr() = default;
};
using ExprPtr = std::unique_ptr<Expr>;

class LiteralExpr : public Expr {
public:
    explicit LiteralExpr(ValuableValue v) : value(std::move(v)) {}
    ValuableValue accept(ExprVisitor& v) override { return v.visitLiteralExpr(*this); }

    const ValuableValue& getValue() const { return value; }

private:
    ValuableValue value;
};

class UnaryExpr : public Expr {
public:
    UnaryExpr(Token op, ExprPtr right) : op(std::move(op)), right(std::move(right)) {}
    ValuableValue accept(ExprVisitor& v) override { return v.visitUnaryExpr(*this); }

    const Token& getOp()    const { return op; }
    ExprPtr& getRight() { return right; }

private:
    Token   op;
    ExprPtr right;
};

class BinaryExpr : public Expr {
public:
    BinaryExpr(ExprPtr l, Token op, ExprPtr r)
        : left(std::move(l)), op(std::move(op)), right(std::move(r)) {
    }
    ValuableValue accept(ExprVisitor& v) override { return v.visitBinaryExpr(*this); }

    ExprPtr& getLeft()  { return left; }
    const Token& getOp()    const { return op; }
    ExprPtr& getRight() { return right; }

private:
    ExprPtr left;
    Token   op;
    ExprPtr right;
};

//( expr )
class GroupingExpr : public Expr {
public:
    explicit GroupingExpr(ExprPtr expr) : expression(std::move(expr)) {}
    ValuableValue accept(ExprVisitor& v) override { return v.visitGroupingExpr(*this); }

    ExprPtr& getExpression() { return expression; }

private:
    ExprPtr expression;
};

class VariableExpr : public Expr {
public:
    explicit VariableExpr(Token name) : name(std::move(name)) {}
    ValuableValue accept(ExprVisitor& v) override { return v.visitVariableExpr(*this); }

    const Token& getName() const { return name; }

private:
    Token name;
};

class AssignExpr : public Expr {
public:
    AssignExpr(Token name, ExprPtr val) : name(std::move(name)), value(std::move(val)) {}
    ValuableValue accept(ExprVisitor& v) override { return v.visitAssignExpr(*this); }

    const Token& getName()  const { return name; }
    ExprPtr& getValue() { return value; }

private:
    Token   name;
    ExprPtr value;
};
