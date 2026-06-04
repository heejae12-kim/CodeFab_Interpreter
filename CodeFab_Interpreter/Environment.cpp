#include "Environment.h"

void Environment::define(const std::string& name, ValuableValue value) {
    values[name] = std::move(value);
}

ValuableValue Environment::get(const Token& name) const {
    auto found = values.find(name.getLexme());
    if (found != values.end()) return found->second;
    if (enclosing)          return enclosing->get(name);
    throw RuntimeError(name, "Undefined variable '" + name.getLexme() + "'.");
}

void Environment::assign(const Token& name, ValuableValue value) {
    auto found = values.find(name.getLexme());
    if (found != values.end()) { found->second = std::move(value); return; }
    if (enclosing)          { enclosing->assign(name, std::move(value)); return; }
    throw RuntimeError(name, "Undefined variable '" + name.getLexme() + "'.");
}
