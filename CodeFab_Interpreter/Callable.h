#pragma once
#include "Token.h"
#include "Stmt.h"
#include "Environment.h"
#include <vector>
#include <memory>
#include <string>

class Interpreter;

// [A안 - Interpreter Pattern]
// 호출 가능한 값이 "스스로 어떻게 실행되는지(call)"를 아는 다형 객체.
// 새 호출 종류(사용자 함수, 내장 함수)는 이 인터페이스를 구현만 하면 된다.
class Callable {
public:
    virtual int           arity() const = 0;
    virtual ValuableValue call(Interpreter& interpreter, std::vector<ValuableValue>& arguments,
                               const Token& paren) = 0;
    virtual std::string   toString() const = 0;
    virtual ~Callable() = default;
};

// return 문 처리를 위한 제어 흐름 예외 (함수 경계까지 unwind)
struct ReturnException {
    ValuableValue value;
};

// 사용자 정의 함수 — 선언(FuncStmt)과 선언 시점 환경(closure)을 캡처해 스스로 실행한다.
class FunctionObject : public Callable {
public:
    FunctionObject(FuncStmt& declaration, std::shared_ptr<Environment> closure)
        : declaration(declaration), closure(std::move(closure)) {
    }
    int           arity() const override { return static_cast<int>(declaration.getParams().size()); }
    ValuableValue call(Interpreter& interpreter, std::vector<ValuableValue>& arguments,
                       const Token& paren) override;
    std::string   toString() const override { return "<fn " + declaration.getName().getLexme() + ">"; }

private:
    FuncStmt&                    declaration;
    std::shared_ptr<Environment> closure;
};

// 내장 함수 Array(n) — 크기 n 배열을 스스로 생성한다.
class NativeArray : public Callable {
public:
    int           arity() const override { return 1; }
    ValuableValue call(Interpreter& interpreter, std::vector<ValuableValue>& arguments,
                       const Token& paren) override;
    std::string   toString() const override { return "<native fn Array>"; }
};
