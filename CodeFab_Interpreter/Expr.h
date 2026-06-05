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
class CallExpr;
class ArrIndexGetExpr;
class ArrIndexSetExpr;

interface ExprVisitor{
    virtual ValuableValue visitLiteralExpr(LiteralExpr & expr) = 0;
    virtual ValuableValue visitUnaryExpr(UnaryExpr& expr) = 0;
    virtual ValuableValue visitBinaryExpr(BinaryExpr& expr) = 0;
    virtual ValuableValue visitGroupingExpr(GroupingExpr& expr) = 0;
    virtual ValuableValue visitVariableExpr(VariableExpr& expr) = 0;
    virtual ValuableValue visitAssignExpr(AssignExpr& expr) = 0;
    virtual ValuableValue visitCallExpr(CallExpr& expr) = 0;
    virtual ValuableValue visitIndexGetExpr(ArrIndexGetExpr& expr) = 0;
    virtual ValuableValue visitIndexSetExpr(ArrIndexSetExpr& expr) = 0;
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

    int          getDistance() const { return distance; }
    void         setDistance(int d) { distance = d; }
private:
    Token name;
    int   distance = -1;
};

class AssignExpr : public Expr {
public:
    AssignExpr(Token name, ExprPtr val) : name(std::move(name)), value(std::move(val)) {}
    ValuableValue accept(ExprVisitor& v) override { return v.visitAssignExpr(*this); }

    const Token& getName()  const { return name; }
    ExprPtr& getValue() { return value; }
    int          getDistance() const { return distance; }
    void         setDistance(int d) { distance = d; }

private:
    Token   name;
    ExprPtr value;
    int     distance = -1;
};

// func(arg1, arg2, ...)
class CallExpr : public Expr {
public:
    CallExpr(ExprPtr callee, Token paren, std::vector<ExprPtr> args)
        : callee(std::move(callee)), paren(std::move(paren)), arguments(std::move(args)) {
    }
    ValuableValue accept(ExprVisitor& v) override { return v.visitCallExpr(*this); }
    ExprPtr& getCallee() { return callee; }
    const Token& getParen()     const { return paren; }
    std::vector<ExprPtr>& getArguments() { return arguments; }
private:
    ExprPtr              callee;
    Token                paren;
    std::vector<ExprPtr> arguments;
};

// object[index]  (read)
class ArrIndexGetExpr : public Expr {
public:
    ArrIndexGetExpr(ExprPtr object, ExprPtr index, Token bracket)
        : object(std::move(object)), index(std::move(index)), bracket(std::move(bracket)) {
    }
    ValuableValue accept(ExprVisitor& v) override { return v.visitIndexGetExpr(*this); }
    ExprPtr& getObject() { return object; }
    ExprPtr& getIndex() { return index; }
    const Token& getBracket() const { return bracket; }
    ExprPtr takeObject() { return std::move(object); }
    ExprPtr takeIndex() { return std::move(index); }
private:
    ExprPtr object;
    ExprPtr index;
    Token   bracket;
};

// object[index] = value  (write)
class ArrIndexSetExpr : public Expr {
public:
    ArrIndexSetExpr(ExprPtr object, ExprPtr index, Token bracket, ExprPtr value)
        : object(std::move(object)), index(std::move(index))
        , bracket(std::move(bracket)), value(std::move(value)) {
    }
    ValuableValue accept(ExprVisitor& v) override { return v.visitIndexSetExpr(*this); }
    ExprPtr& getObject() { return object; }
    ExprPtr& getIndex() { return index; }
    const Token& getBracket() const { return bracket; }
    ExprPtr& getValue() { return value; }
private:
    ExprPtr object;
    ExprPtr index;
    Token   bracket;
    ExprPtr value;
};
