#pragma once
#include <string>
#include <variant>
#include <vector>
#include <memory>

enum class TokenType {
    // Single-char tokens
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET,
    SEMICOLON,
    PLUS, MINUS, STAR, SLASH,
    COMMA,

    // One or two char tokens
    LESS, LESS_EQUAL,
    GREATER, GREATER_EQUAL,
    EQUAL, EQUAL_EQUAL,
    BANG_EQUAL, 

    // Literals
    NUMBER, STRING, IDENTIFIER,

    // Keywords
    VAR, IF, ELSE, FOR, PRINT,
    TRUE_KW, FALSE_KW,
    FUNC, RETURN,
    ARRAY,

    // Boolean Operator
    AND_OP, OR_OP, BANG,

    EOF_TOKEN
};

// 배열 값 타입 — ValuableValue의 재귀 구조를 위해 forward declaration 사용
class ArrayValue;
using ArrayPtr = std::shared_ptr<ArrayValue>;
// [A안] 호출 가능한 값(함수 등) — 정의는 Callable.h, 여기서는 전방선언만 둔다.
class Callable;
using CallablePtr = std::shared_ptr<Callable>;

// Runtime value type: number, string, bool, nil, array, callable
using ValuableValue = std::variant<double, std::string, bool, std::nullptr_t, ArrayPtr, CallablePtr>;

class ArrayValue {
public:
    std::vector<ValuableValue> elements;
    explicit ArrayValue(std::size_t size) : elements(size, nullptr) {}
};

class Token {
public:
    Token(TokenType type, std::string lexeme, ValuableValue literal, int line)
        : type(type), lexeme(std::move(lexeme)), literal(std::move(literal)), line(line) {
    }

    TokenType getTokenType() const { return type; }
    std::string getLexme() const { return lexeme; }
    ValuableValue getLiteral() const { return literal; }
    int getLine() const { return line; }

private:
    TokenType   type;
    std::string lexeme;
    ValuableValue       literal;
    int         line;
};