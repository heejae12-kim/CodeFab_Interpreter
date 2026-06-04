#include "Lexer.h"

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

std::vector<Token> Lexer::tokenize() {
    // TODO: RED → GREEN 단계에서 구현
    return {};
}

bool Lexer::isAtEnd() const {
    return current_ >= static_cast<int>(source_.size());
}

char Lexer::advance() {
    return source_[current_++];
}

char Lexer::peek() const {
    return isAtEnd() ? '\0' : source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= static_cast<int>(source_.size())) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source_[current_] != expected) return false;
    ++current_;
    return true;
}

void Lexer::scanToken() {}
void Lexer::scanString() {}
void Lexer::scanNumber() {}
void Lexer::scanIdentifierOrKeyword() {}

void Lexer::addToken(TokenType type) {
    addToken(type, nullptr);
}

void Lexer::addToken(TokenType type, ValuableValue literal) {
    std::string text = source_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, std::move(text), std::move(literal), line_);
}
