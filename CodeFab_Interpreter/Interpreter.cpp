#include "Interpreter.h"
#include "Callable.h"     // [A안] Callable / FunctionObject / NativeArray / ReturnException
#include <iostream>
#include <sstream>
#include <cmath>

Interpreter::Interpreter() {
    globalEnv  = std::make_shared<Environment>();
    currentEnv = globalEnv;
    // [A안] 내장 함수도 일반 호출 가능한 값으로 전역에 등록 → 호출 경로가 통일된다.
    globalEnv->define("Array", std::make_shared<NativeArray>());
}

void Interpreter::interpret(const std::vector<StmtPtr>& statements) {
    for (const auto& stmt : statements) execute(*stmt);
}

void Interpreter::executeSingleStmt(Stmt& stmt) { execute(stmt); }

void Interpreter::setStmtHook(StmtHook hook) { stmtHook_ = std::move(hook); }
void Interpreter::clearStmtHook()             { stmtHook_ = nullptr; }

void Interpreter::execute(Stmt& stmt) {
    if (stmtHook_) stmtHook_(stmt, execDepth_);
    stmt.accept(*this);
}
ValuableValue Interpreter::evaluate(Expr& expr) { return expr.accept(*this); }

// ── Utilities ──────────────────────────────────────────────────────────────────

bool Interpreter::isTruthy(const ValuableValue& v) {
    if (std::holds_alternative<std::nullptr_t>(v)) return false;
    if (std::holds_alternative<bool>(v))           return std::get<bool>(v);
    return true;
}

// 거짓 같은 값(nil 또는 false)인지 판별한다.
bool Interpreter::isFalse(const ValuableValue& v) {
    return !isTruthy(v);
}

bool Interpreter::isEqual(const ValuableValue& a, const ValuableValue& b) {
    if (a.index() != b.index()) return false;
    if (std::holds_alternative<std::nullptr_t>(a)) return true;
    if (std::holds_alternative<bool>(a))   return std::get<bool>(a)   == std::get<bool>(b);
    if (std::holds_alternative<double>(a)) return std::get<double>(a) == std::get<double>(b);
    if (std::holds_alternative<std::string>(a))
        return std::get<std::string>(a) == std::get<std::string>(b);
    if (std::holds_alternative<ArrayPtr>(a))
        return std::get<ArrayPtr>(a) == std::get<ArrayPtr>(b);   // 포인터 동일성
    if (std::holds_alternative<CallablePtr>(a))
        return std::get<CallablePtr>(a) == std::get<CallablePtr>(b);
    return false;
}

std::string Interpreter::stringify(const ValuableValue& v) {
    if (std::holds_alternative<std::nullptr_t>(v)) return "nil";
    if (std::holds_alternative<bool>(v))  return std::get<bool>(v) ? "true" : "false";
    if (std::holds_alternative<std::string>(v)) return std::get<std::string>(v);
    if (std::holds_alternative<double>(v)) {
        double d = std::get<double>(v);
        // 정수면 소수점 없이 출력 (5.0 → "5")
        if (!std::isinf(d) && d == std::floor(d))
            return std::to_string(static_cast<long long>(d));
        std::ostringstream oss;
        oss << d;
        return oss.str();
    }
    if (std::holds_alternative<ArrayPtr>(v)) {
        const auto& elements = std::get<ArrayPtr>(v)->elements;
        std::string out = "[";
        for (std::size_t i = 0; i < elements.size(); ++i) {
            if (i) out += ", ";
            out += stringify(elements[i]);
        }
        return out + "]";
    }
    if (std::holds_alternative<CallablePtr>(v))
        return std::get<CallablePtr>(v)->toString();
    return "";
}

void Interpreter::checkNumberOperand(const Token& op, const ValuableValue& operand) {
    if (std::holds_alternative<double>(operand)) return;
    throw RuntimeError(op, "Operand must be a number.");
}

void Interpreter::checkNumberOperands(const Token& op, const ValuableValue& left, const ValuableValue& right) {
    if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) return;
    throw RuntimeError(op, "Operands must be numbers.");
}

void Interpreter::executeBlock(const std::vector<StmtPtr>& stmts,
                               std::shared_ptr<Environment> env) {
    ++execDepth_;
    auto saved = currentEnv;
    currentEnv = std::move(env);
    try {
        for (const auto& s : stmts) execute(*s);
    } catch (...) {
        currentEnv = saved;
        --execDepth_;
        throw;
    }
    currentEnv = saved;
    --execDepth_;
}

// ── ExprVisitor ────────────────────────────────────────────────────────────────

ValuableValue Interpreter::visitLiteralExpr(LiteralExpr& expr) { return expr.getValue(); }

ValuableValue Interpreter::visitGroupingExpr(GroupingExpr& expr) {
    return evaluate(*expr.getExpression());
}

ValuableValue Interpreter::visitUnaryExpr(UnaryExpr& expr) {
    ValuableValue right = evaluate(*expr.getRight());
    switch (expr.getOp().getTokenType()) {
    case TokenType::MINUS:
        checkNumberOperand(expr.getOp(), right);
        return -std::get<double>(right);
    case TokenType::BANG:
        return isFalse(right);   // 논리 부정: 거짓 같은 값이면 true
    default:
        return nullptr;
    }
}

ValuableValue Interpreter::visitBinaryExpr(BinaryExpr& expr) {
    // 논리 연산자(and/or)는 단축 평가(short-circuit)를 위해 먼저 처리한다.
    if (expr.getOp().getTokenType() == TokenType::AND_OP) {
        ValuableValue left = evaluate(*expr.getLeft());
        if (isFalse(left))            // 좌변이 거짓이면 우변 평가 없이 반환
            return left;
        return evaluate(*expr.getRight());
    }
    if (expr.getOp().getTokenType() == TokenType::OR_OP) {
        ValuableValue left = evaluate(*expr.getLeft());
        if (isTruthy(left))
            return left;
        return evaluate(*expr.getRight());
    }

    ValuableValue left  = evaluate(*expr.getLeft());
    ValuableValue right = evaluate(*expr.getRight());

    switch (expr.getOp().getTokenType()) {
    case TokenType::PLUS:
        if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
            return std::get<double>(left) + std::get<double>(right);
        if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
            return std::get<std::string>(left) + std::get<std::string>(right);
        throw RuntimeError(expr.getOp(), "Operands must be two numbers or two strings.");
    case TokenType::MINUS:
        checkNumberOperands(expr.getOp(), left, right);
        return std::get<double>(left) - std::get<double>(right);
    case TokenType::STAR:
        checkNumberOperands(expr.getOp(), left, right);
        return std::get<double>(left) * std::get<double>(right);
    case TokenType::SLASH:
        checkNumberOperands(expr.getOp(), left, right);
        if (std::get<double>(right) == 0.0)
            throw RuntimeError(expr.getOp(), "Division by zero.");
        return std::get<double>(left) / std::get<double>(right);
    case TokenType::LESS:
        checkNumberOperands(expr.getOp(), left, right);
        return std::get<double>(left) < std::get<double>(right);
    case TokenType::LESS_EQUAL:
        checkNumberOperands(expr.getOp(), left, right);
        return std::get<double>(left) <= std::get<double>(right);
    case TokenType::GREATER:
        checkNumberOperands(expr.getOp(), left, right);
        return std::get<double>(left) > std::get<double>(right);
    case TokenType::GREATER_EQUAL:
        checkNumberOperands(expr.getOp(), left, right);
        return std::get<double>(left) >= std::get<double>(right);
    case TokenType::EQUAL_EQUAL:
        return isEqual(left, right);
    case TokenType::BANG_EQUAL:
        return !isEqual(left, right);
    default:
        break;
    }
    return nullptr;
}

ValuableValue Interpreter::visitVariableExpr(VariableExpr& expr) {
    return currentEnv->get(expr.getName());
}

ValuableValue Interpreter::visitAssignExpr(AssignExpr& expr) {
    ValuableValue val = evaluate(*expr.getValue());
    currentEnv->assign(expr.getName(), val);
    return val;
}

// [A안 - Interpreter Pattern] 호출 경로가 통일된다.
//  - callee를 평가해 호출 가능한 값(Callable)을 얻고, 그 객체가 스스로 실행한다.
//  - 함수/내장 Array 구분을 위한 절차적 분기가 없다.
ValuableValue Interpreter::visitCallExpr(CallExpr& expr) {
    ValuableValue callee = evaluate(*expr.getCallee());
    if (!std::holds_alternative<CallablePtr>(callee))
        throw RuntimeError(expr.getParen(), "Can only call functions.");
    CallablePtr callable = std::get<CallablePtr>(callee);

    std::vector<ValuableValue> arguments;
    for (auto& argument : expr.getArguments())
        arguments.push_back(evaluate(*argument));

    if (static_cast<int>(arguments.size()) != callable->arity())
        throw RuntimeError(expr.getParen(),
            "Expected " + std::to_string(callable->arity()) +
            " arguments but got " + std::to_string(arguments.size()) + ".");

    return callable->call(*this, arguments, expr.getParen());
}

ValuableValue Interpreter::visitIndexGetExpr(ArrIndexGetExpr& expr) {
    ValuableValue object = evaluate(*expr.getObject());
    if (!std::holds_alternative<ArrayPtr>(object))
        throw RuntimeError(expr.getBracket(), "Only arrays support index access.");
    ArrayPtr array = std::get<ArrayPtr>(object);

    ValuableValue index = evaluate(*expr.getIndex());
    if (!std::holds_alternative<double>(index))
        throw RuntimeError(expr.getBracket(), "Array index must be a number.");
    int i = static_cast<int>(std::get<double>(index));
    if (i < 0 || i >= static_cast<int>(array->elements.size()))
        throw RuntimeError(expr.getBracket(), "Array index out of range.");

    return array->elements[i];
}

ValuableValue Interpreter::visitIndexSetExpr(ArrIndexSetExpr& expr) {
    ValuableValue object = evaluate(*expr.getObject());
    if (!std::holds_alternative<ArrayPtr>(object))
        throw RuntimeError(expr.getBracket(), "Only arrays support index access.");
    ArrayPtr array = std::get<ArrayPtr>(object);

    ValuableValue index = evaluate(*expr.getIndex());
    if (!std::holds_alternative<double>(index))
        throw RuntimeError(expr.getBracket(), "Array index must be a number.");
    int i = static_cast<int>(std::get<double>(index));
    if (i < 0 || i >= static_cast<int>(array->elements.size()))
        throw RuntimeError(expr.getBracket(), "Array index out of range.");

    ValuableValue value = evaluate(*expr.getValue());
    array->elements[i] = value;
    return value;
}

// ── StmtVisitor ────────────────────────────────────────────────────────────────

void Interpreter::visitPrintStmt(PrintStmt& stmt) {
    std::cout << stringify(evaluate(*stmt.getExpression())) << "\n";
}

void Interpreter::visitExprStmt(ExprStmt& stmt) {
    evaluate(*stmt.getExpression());
}

void Interpreter::visitVarStmt(VarStmt& stmt) {
    ValuableValue val = nullptr;
    if (stmt.getInitializer()) val = evaluate(*stmt.getInitializer());
    currentEnv->define(stmt.getName().getLexme(), std::move(val));
}

void Interpreter::visitBlockStmt(BlockStmt& stmt) {
    executeBlock(stmt.getStatements(), std::make_shared<Environment>(currentEnv));
}

void Interpreter::visitIfStmt(IfStmt& stmt) {
    if (isTruthy(evaluate(*stmt.getCondition())))
        execute(*stmt.getThenBranch());
    else if (stmt.getElseBranch())
        execute(*stmt.getElseBranch());
}

void Interpreter::visitForStmt(ForStmt& stmt) {
    // for 초기화 변수를 위한 새 스코프
    auto loopEnv = std::make_shared<Environment>(currentEnv);
    auto saved   = currentEnv;
    currentEnv   = loopEnv;
    try {
        if (stmt.getInitializer()) execute(*stmt.getInitializer());
        while (true) {
            if (stmt.getCondition() && !isTruthy(evaluate(*stmt.getCondition()))) break;
            execute(*stmt.getBody());
            if (stmt.getIncrement()) evaluate(*stmt.getIncrement());
        }
    } catch (...) {
        currentEnv = saved;
        throw;
    }
    currentEnv = saved;
}

// [A안] 함수 선언은 호출 가능한 "값"(FunctionObject)으로 만들어 환경에 정의한다.
//       선언 시점 환경을 클로저로 캡처 → 1급 함수/재귀 자연 지원.
void Interpreter::visitFuncStmt(FuncStmt& stmt) {
    auto function = std::make_shared<FunctionObject>(stmt, currentEnv);
    currentEnv->define(stmt.getName().getLexme(), function);
}

void Interpreter::visitReturnStmt(ReturnStmt& stmt) {
    ValuableValue value = nullptr;
    if (stmt.getValue()) value = evaluate(*stmt.getValue());
    throw ReturnException{ std::move(value) };
}
