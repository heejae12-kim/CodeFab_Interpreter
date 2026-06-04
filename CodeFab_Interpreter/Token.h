#pragma once
#include <string>
#include <variant>

enum class TokenType {
    // Single-char tokens
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    SEMICOLON,
    PLUS, MINUS, STAR, SLASH,

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

    EOF_TOKEN
};

// Runtime value type: number, string, bool, or nil
using ValuableValue = std::variant<double, std::string, bool, std::nullptr_t>;

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