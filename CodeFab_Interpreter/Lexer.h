#pragma once
#include "Token.h"
#include <string>
#include <vector>
#include <stdexcept>

class LexError : public std::runtime_error {
public:
    LexError(int line, const std::string& msg)
        : std::runtime_error("[line " + std::to_string(line) + "] Syntax Error: " + msg)
        , line_(line) {
    }
    int line() const { return line_; }
private:
    int line_;
};

class Lexer {
public:
    explicit Lexer(std::string source);

    // 소스 문자열을 Token 배열로 변환한다. 마지막 원소는 항상 EOF_TOKEN.
    // 어휘 오류 발생 시 LexError를 throw한다.
    std::vector<Token> tokenize();

private:
    std::string        source_;
    std::vector<Token> tokens_;
    int                start_   = 0;
    int                current_ = 0;
    int                line_    = 1;

    bool isAtEnd()    const;
    char advance();
    char peek()       const;
    char peekNext()   const;
    bool match(char expected);

    void scanToken();
    void scanString();
    void scanNumber();
    void scanIdentifierOrKeyword();

    void addToken(TokenType type);
    void addToken(TokenType type, ValuableValue literal);
};
