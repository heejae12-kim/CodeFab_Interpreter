#pragma once
#include "Token.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

// ── Runtime Error ──────────────────────────────────────────────────────────────
// 실행 중 발생하는 오류. 오류 위치 보고를 위해 원인 Token 을 함께 보관한다.
struct RuntimeError : std::runtime_error {
    Token token;
    RuntimeError(Token tok, const std::string& msg)
        : std::runtime_error(msg), token(std::move(tok)) {}
};

// ── Lexical Environment (scope chain) ─────────────────────────────────────────
// 변수의 이름과 값을 보관하는 저장소. enclosing 으로 상위 스코프를 가리켜
// 블록 단위 스코프 체인을 형성한다. (현재 → 상위 → ... → Global)
class Environment {
public:
    std::shared_ptr<Environment> enclosing;

    explicit Environment(std::shared_ptr<Environment> enc = nullptr)
        : enclosing(std::move(enc)) {}

    void          define(const std::string& name, ValuableValue value);
    ValuableValue get   (const Token& name) const;
    void          assign(const Token& name, ValuableValue value);

private:
    std::unordered_map<std::string, ValuableValue> values;
};
