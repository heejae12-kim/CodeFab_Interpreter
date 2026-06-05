#include "Parser.h"
#include "gmock/gmock.h"

class MockStmtVisitor : public StmtVisitor {
public:
    MOCK_METHOD(void, visitPrintStmt,  (PrintStmt&),  (override));
    MOCK_METHOD(void, visitExprStmt,   (ExprStmt&),   (override));
    MOCK_METHOD(void, visitVarStmt,    (VarStmt&),    (override));
    MOCK_METHOD(void, visitBlockStmt,  (BlockStmt&),  (override));
    MOCK_METHOD(void, visitIfStmt,     (IfStmt&),     (override));
    MOCK_METHOD(void, visitForStmt,    (ForStmt&),    (override));
    MOCK_METHOD(void, visitFuncStmt,   (FuncStmt&),   (override));
    MOCK_METHOD(void, visitReturnStmt, (ReturnStmt&), (override));
};

class ParseErrorFixture : public testing::Test {
protected:
    void expectParseError(std::vector<Token> tokens, const std::string& expectedMsg = "") {
        Parser parser(std::move(tokens));
        try {
            parser.parse();
            ADD_FAILURE() << "Expected ParseError to be thrown";
        } catch (const ParseError& e) {
            if (!expectedMsg.empty())
                EXPECT_THAT(std::string(e.what()), testing::HasSubstr(expectedMsg));
        }
    }
};

TEST_F(ParseErrorFixture, VarMissingIdentifier) {
    expectParseError({
        Token(TokenType::VAR,       "var", nullptr, 1),
        Token(TokenType::EQUAL,     "=",   nullptr, 1),  // IDENTIFIER 누락
        Token(TokenType::NUMBER,    "10",  10.0,    1),
        Token(TokenType::SEMICOLON, ";",   nullptr, 1),
        Token(TokenType::EOF_TOKEN, "",    nullptr, 1),
    }, "Expected variable name.");
}

TEST_F(ParseErrorFixture, VarMissingSemicolon) {
    expectParseError({
        Token(TokenType::VAR,        "var", nullptr, 1),
        Token(TokenType::IDENTIFIER, "a",   nullptr, 1),
        Token(TokenType::EQUAL,      "=",   nullptr, 1),
        Token(TokenType::NUMBER,     "10",  10.0,    1),
        // SEMICOLON 누락
        Token(TokenType::EOF_TOKEN,  "",    nullptr, 1),
    }, "Expected ';' after variable declaration.");
}

TEST_F(ParseErrorFixture, PrintMissingSemicolon) {
    expectParseError({
        Token(TokenType::PRINT,     "print", nullptr, 1),
        Token(TokenType::NUMBER,    "10",    10.0,    1),
        // SEMICOLON 누락
        Token(TokenType::EOF_TOKEN, "",      nullptr, 1),
    }, "Expected ';' after value.");
}

// if a > 1) { print 1; }
TEST_F(ParseErrorFixture, IfMissingLeftParen) {
    expectParseError({
        Token(TokenType::IF,          "if",    nullptr, 1),
        // LEFT_PAREN 누락
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::GREATER,     ">",     nullptr, 1),
        Token(TokenType::NUMBER,      "1",     1.0,     1),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 1),
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 1),
        Token(TokenType::PRINT,       "print", nullptr, 1),
        Token(TokenType::NUMBER,      "1",     1.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 1),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 1),
    }, "Expected '(' after 'if'.");
}

// if (a > 1 { print 1; }
TEST_F(ParseErrorFixture, IfMissingRightParen) {
    expectParseError({
        Token(TokenType::IF,          "if",    nullptr, 1),
        Token(TokenType::LEFT_PAREN,  "(",     nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::GREATER,     ">",     nullptr, 1),
        Token(TokenType::NUMBER,      "1",     1.0,     1),
        // RIGHT_PAREN 누락
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 1),
        Token(TokenType::PRINT,       "print", nullptr, 1),
        Token(TokenType::NUMBER,      "1",     1.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 1),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 1),
    }, "Expected ')' after if condition.");
}

// { print 1;
TEST_F(ParseErrorFixture, BlockMissingRightBrace) {
    expectParseError({
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 1),
        Token(TokenType::PRINT,       "print", nullptr, 1),
        Token(TokenType::NUMBER,      "1",     1.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        // RIGHT_BRACE 누락
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 1),
    }, "Expected '}' after block.");
}

// for a = 0; a < 4; a = a + 1) { ... }
TEST_F(ParseErrorFixture, ForMissingLeftParen) {
    expectParseError({
        Token(TokenType::FOR,         "for",   nullptr, 1),
        // LEFT_PAREN 누락
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::EQUAL,       "=",     nullptr, 1),
        Token(TokenType::NUMBER,      "0",     0.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::LESS,        "<",     nullptr, 1),
        Token(TokenType::NUMBER,      "4",     4.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::EQUAL,       "=",     nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::PLUS,        "+",     nullptr, 1),
        Token(TokenType::NUMBER,      "1",     1.0,     1),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 1),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 1),
    }, "Expected '(' after 'for'.");
}

// if (a > 1) print 3; }  → thenBranch는 파싱되고 } 가 statement로 오면 "Expected expression."
TEST_F(ParseErrorFixture, IfMissingLeftBrace) {
    expectParseError({
        Token(TokenType::IF,          "if",    nullptr, 1),
        Token(TokenType::LEFT_PAREN,  "(",     nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::GREATER,     ">",     nullptr, 1),
        Token(TokenType::NUMBER,      "1",     1.0,     1),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 1),
        // LEFT_BRACE 누락
        Token(TokenType::PRINT,       "print", nullptr, 1),
        Token(TokenType::NUMBER,      "3",     3.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 1),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 1),
    }, "Expected expression.");
}

// for body에서 { 누락 → } 가 statement로 오면 "Expected expression."
TEST_F(ParseErrorFixture, ForVarInitUnexpectedRightBrace) {
    expectParseError({
        Token(TokenType::FOR,         "for",   nullptr, 1),
        Token(TokenType::LEFT_PAREN,  "(",     nullptr, 1),
        Token(TokenType::VAR,         "var",   nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::EQUAL,       "=",     nullptr, 1),
        Token(TokenType::NUMBER,      "0",     0.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::LESS,        "<",     nullptr, 1),
        Token(TokenType::NUMBER,      "4",     4.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::EQUAL,       "=",     nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::PLUS,        "+",     nullptr, 1),
        Token(TokenType::NUMBER,      "1",     1.0,     1),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 1),
        Token(TokenType::PRINT,       "print", nullptr, 2),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 2),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 2),
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 3),  // { 없는 }
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 3),
    }, "Expected expression.");
}

// print arr[0; → 인덱스 읽기에서 ] 누락
TEST_F(ParseErrorFixture, ArrayMissingRightBracketOnRead) {
    expectParseError({
        Token(TokenType::PRINT,        "print", nullptr, 1),
        Token(TokenType::IDENTIFIER,   "arr",   nullptr, 1),
        Token(TokenType::LEFT_BRACKET, "[",     nullptr, 1),
        Token(TokenType::NUMBER,       "0",     0.0,     1),
        // RIGHT_BRACKET 누락
        Token(TokenType::SEMICOLON,    ";",     nullptr, 1),
        Token(TokenType::EOF_TOKEN,    "",      nullptr, 1),
    }, "Expected ']' after index.");
}

// arr[0 = 5; → ] 누락으로 인해 index 자리의 0이 대입 대상으로 파싱됨
TEST_F(ParseErrorFixture, ArrayMissingRightBracketOnWrite) {
    expectParseError({
        Token(TokenType::IDENTIFIER,   "arr",   nullptr, 1),
        Token(TokenType::LEFT_BRACKET, "[",     nullptr, 1),
        Token(TokenType::NUMBER,       "0",     0.0,     1),
        // RIGHT_BRACKET 누락 → assignment()가 0 = 5 로 파싱 시도
        Token(TokenType::EQUAL,        "=",     nullptr, 1),
        Token(TokenType::NUMBER,       "5",     5.0,     1),
        Token(TokenType::SEMICOLON,    ";",     nullptr, 1),
        Token(TokenType::EOF_TOKEN,    "",      nullptr, 1),
    }, "Invalid assignment target.");
}

// add(x) = 5; → 함수 호출 결과에 대입 시도
TEST_F(ParseErrorFixture, InvalidAssignmentTarget) {
    expectParseError({
        Token(TokenType::IDENTIFIER,  "add", nullptr, 1),
        Token(TokenType::LEFT_PAREN,  "(",   nullptr, 1),
        Token(TokenType::IDENTIFIER,  "x",   nullptr, 1),
        Token(TokenType::RIGHT_PAREN, ")",   nullptr, 1),
        Token(TokenType::EQUAL,       "=",   nullptr, 1),
        Token(TokenType::NUMBER,      "5",   5.0,     1),
        Token(TokenType::SEMICOLON,   ";",   nullptr, 1),
        Token(TokenType::EOF_TOKEN,   "",    nullptr, 1),
    }, "Invalid assignment target.");
}
