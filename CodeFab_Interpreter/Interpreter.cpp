#include "Interpreter.h"
#include <iostream>
#include <sstream>
#include <cmath>

// return 문 처리를 위한 제어 흐름 예외 (호출 스택을 함수 경계까지 unwind)
namespace {
    struct ReturnException {
        ValuableValue value;
    };
}

Interpreter::Interpreter() {
    globalEnv  = std::make_shared<Environment>();
    currentEnv = globalEnv;
}

void Interpreter::interpret(const std::vector<StmtPtr>& statements) {
    for (const auto& stmt : statements) execute(*stmt);
}

void Interpreter::execute(Stmt& stmt) { stmt.accept(*this); }
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
    auto saved = currentEnv;
    currentEnv = std::move(env);
    try {
        for (const auto& s : stmts) execute(*s);
    } catch (...) {
        currentEnv = saved;
        throw;
    }
    currentEnv = saved;
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

// [B안] 호출은 인터프리터가 중앙에서 절차적으로 분기한다.
//  - 호출 대상은 반드시 이름(VariableExpr) 이어야 한다 (함수는 1급 값이 아님).
//  - 내장 Array(n) 와 사용자 정의 함수(함수표)를 if/lookup 으로 구분한다.
ValuableValue Interpreter::visitCallExpr(CallExpr& expr) {
    auto* callee_name = dynamic_cast<VariableExpr*>(expr.getCallee().get());
    if (!callee_name)
        throw RuntimeError(expr.getParen(), "Can only call named functions.");
    const std::string name = callee_name->getName().getLexme();

    std::vector<ValuableValue> arguments;
    for (auto& argument : expr.getArguments())
        arguments.push_back(evaluate(*argument));

    if (name == "Array")                       // 내장 배열 생성
        return makeArray(arguments, expr.getParen());

    auto found = functions_.find(name);        // 사용자 정의 함수
    if (found == functions_.end())
        throw RuntimeError(expr.getParen(), "'" + name + "' is not a function.");
    return callFunction(*found->second, arguments, expr.getParen());
}

ValuableValue Interpreter::callFunction(FuncStmt& fn, std::vector<ValuableValue>& args,
                                        const Token& paren) {
    if (args.size() != fn.getParams().size())
        throw RuntimeError(paren, "Expected " + std::to_string(fn.getParams().size()) +
            " arguments but got " + std::to_string(args.size()) + ".");

    auto env = std::make_shared<Environment>(globalEnv); // B안: 함수는 전역만 본다(클로저 없음)
    const auto& params = fn.getParams();
    for (std::size_t i = 0; i < params.size(); ++i)
        env->define(params[i].getLexme(), args[i]);

    try {
        executeBlock(fn.getBody(), env);
    }
    catch (const ReturnException& returned) {
        return returned.value;
    }
    return nullptr; // return 문이 없으면 nil
}

ValuableValue Interpreter::makeArray(std::vector<ValuableValue>& args, const Token& paren) {
    if (args.size() != 1)
        throw RuntimeError(paren, "Array expects exactly 1 argument (size).");
    if (!std::holds_alternative<double>(args[0]))
        throw RuntimeError(paren, "Array size must be a number.");
    double size = std::get<double>(args[0]);
    if (size < 0)
        throw RuntimeError(paren, "Array size must not be negative.");
    return std::make_shared<ArrayValue>(static_cast<std::size_t>(size));
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

// [B안] 함수 선언은 인터프리터의 함수표에 등록만 한다 (값으로 만들지 않음).
void Interpreter::visitFuncStmt(FuncStmt& stmt) {
    functions_[stmt.getName().getLexme()] = &stmt;
}

void Interpreter::visitReturnStmt(ReturnStmt& stmt) {
    ValuableValue value = nullptr;
    if (stmt.getValue()) value = evaluate(*stmt.getValue());
    throw ReturnException{ std::move(value) };
}
