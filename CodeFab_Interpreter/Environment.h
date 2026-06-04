#pragma once
#include "Token.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

struct RuntimeError : std::runtime_error {
    Token token;
    RuntimeError(Token tok, const std::string& msg)
        : std::runtime_error(msg), token(std::move(tok)) {}
};


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
