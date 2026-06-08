#pragma once
#include "Token.h"
#include <unordered_map>
#include <optional>
#include <string>
#include <memory>
#include <stdexcept>

class RuntimeError : public std::runtime_error {
public:
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
    std::optional<ValuableValue> tryGet(const std::string& name) const;
    const std::unordered_map<std::string, ValuableValue>& getValues() const { return values; }

private:
    std::unordered_map<std::string, ValuableValue> values;
};
