#include "Lexer.h"
#include <cctype>
#include <unordered_map>

static const std::unordered_map<std::string, TokenType> keywords = {
    {"var",    TokenType::VAR},
    {"if",     TokenType::IF},
    {"else",   TokenType::ELSE},
    {"for",    TokenType::FOR},
    {"print",  TokenType::PRINT},
    {"true",   TokenType::TRUE_KW},
    {"false",  TokenType::FALSE_KW},
    {"Func",   TokenType::FUNC},   // 함수 선언 키워드 (PDF 원문 기준 대문자 F)
    {"return", TokenType::RETURN}, // 반환문 키워드
    {"Array",  TokenType::ARRAY},  // 배열 생성 키워드 (PDF 원문 기준 대문자 A)
    {"and",    TokenType::AND_OP}, // 논리 AND 연산자
    {"or",     TokenType::OR_OP},  // 논리 OR 연산자
};

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) {
        start_ = current_;
        scanToken();
    }
    tokens_.emplace_back(TokenType::EOF_TOKEN, "", nullptr, line_);
    return std::move(tokens_);
}

bool Lexer::isAtEnd() const {
    return current_ >= static_cast<int>(source_.size());
}

char Lexer::advance() {
    return source_[current_++];
}

char Lexer::peek() const {
    return isAtEnd() ? NULL_CHARACTER : source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= static_cast<int>(source_.size())) return NULL_CHARACTER;
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source_[current_] != expected) return false;
    ++current_;
    return true;
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
    case '(': addToken(TokenType::LEFT_PAREN);  break;
    case ')': addToken(TokenType::RIGHT_PAREN); break;
    case '{': addToken(TokenType::LEFT_BRACE);  break;
    case '}': addToken(TokenType::RIGHT_BRACE); break;
    case '[': addToken(TokenType::LEFT_BRACKET);  break;
    case ']': addToken(TokenType::RIGHT_BRACKET); break;
    case ';': addToken(TokenType::SEMICOLON);     break;
    case ',': addToken(TokenType::COMMA);         break;
    case '+': addToken(TokenType::PLUS);        break;
    case '-': addToken(TokenType::MINUS);       break;
    case '*': addToken(TokenType::STAR);        break;
    case '/': scanSlashOrComment(); break;
    case '<': addToken(match('=') ? TokenType::LESS_EQUAL    : TokenType::LESS);    break;
    case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
    case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL   : TokenType::EQUAL);   break;
    case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
    case '"': scanString(); break;
    case ' ': case '\r': case '\t': break;
    case '\n': ++line_; break;
    default:
        if (std::isdigit(static_cast<unsigned char>(c))) {
            scanNumber();
        } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            scanIdentifierOrKeyword();
        } else {
            throw LexError(line_, std::string("unexpected character '") + c + "'");
        }
        break;
    }
}

void Lexer::scanSlashOrComment() {
    if (match('/')) {
        while (!isAtEnd() && peek() != '\n') advance();
    } else {
        addToken(TokenType::SLASH);
    }
}

void Lexer::scanString() {
    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') ++line_;
        advance();
    }
    if (isAtEnd()) {
        throw LexError(line_, "unterminated string");
    }
    advance(); // 닫는 "
    std::string value = source_.substr(start_ + 1, current_ - start_ - 2);
    addToken(TokenType::STRING, std::move(value));
}

void Lexer::scanNumber() {
    while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) advance();
    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
        advance(); // '.'
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) advance();
    }
    double value = std::stod(source_.substr(start_, current_ - start_));
    addToken(TokenType::NUMBER, value);
}

void Lexer::scanIdentifierOrKeyword() {
    while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) advance();
    std::string text = source_.substr(start_, current_ - start_);
    auto found_token = keywords.find(text);
    if (found_token == keywords.end()) {
        addToken(TokenType::IDENTIFIER);
        return;
    }
    TokenType type = found_token->second;
    if (type == TokenType::TRUE_KW)       addToken(type, true);
    else if (type == TokenType::FALSE_KW) addToken(type, false);
    else                                  addToken(type);
}

void Lexer::addToken(TokenType type, ValuableValue literal) {
    std::string text = source_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, std::move(text), std::move(literal), line_);
}
