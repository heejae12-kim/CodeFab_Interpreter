#include "Parser.h"
#include "gmock/gmock.h"

class MockStmtVisitor : public StmtVisitor {
public:
    MOCK_METHOD(void, visitPrintStmt, (PrintStmt&), (override));
    MOCK_METHOD(void, visitExprStmt, (ExprStmt&), (override));
    MOCK_METHOD(void, visitVarStmt, (VarStmt&), (override));
    MOCK_METHOD(void, visitBlockStmt, (BlockStmt&), (override));
    MOCK_METHOD(void, visitIfStmt, (IfStmt&), (override));
    MOCK_METHOD(void, visitForStmt, (ForStmt&), (override));
};

TEST(ParserErrorTest, VarMissingIdentifier) {
    std::vector<Token> tokens = {
        Token(TokenType::VAR,       "var", nullptr, 1),
        Token(TokenType::EQUAL,     "=",   nullptr, 1),  // IDENTIFIER 누락
        Token(TokenType::NUMBER,    "10",  10.0,    1),
        Token(TokenType::SEMICOLON, ";",   nullptr, 1),
        Token(TokenType::EOF_TOKEN, "",    nullptr, 1),
    };
    Parser parser(tokens);
    EXPECT_THROW(parser.parse(), ParseError);
}

// 2. var 선언에서 SEMICOLON 누락: `var a = 10`
TEST(ParserErrorTest, VarMissingSemicolon) {
    std::vector<Token> tokens = {
        Token(TokenType::VAR,        "var", nullptr, 1),
        Token(TokenType::IDENTIFIER, "a",   nullptr, 1),
        Token(TokenType::EQUAL,      "=",   nullptr, 1),
        Token(TokenType::NUMBER,     "10",  10.0,    1),
        // SEMICOLON 누락
        Token(TokenType::EOF_TOKEN,  "",    nullptr, 1),
    };
    Parser parser(tokens);
    EXPECT_THROW(parser.parse(), ParseError);
}

// 3. print에서 SEMICOLON 누락: `print 10`
TEST(ParserErrorTest, PrintMissingSemicolon) {
    std::vector<Token> tokens = {
        Token(TokenType::PRINT,     "print", nullptr, 1),
        Token(TokenType::NUMBER,    "10",    10.0,    1),
        // SEMICOLON 누락
        Token(TokenType::EOF_TOKEN, "",      nullptr, 1),
    };
    Parser parser(tokens);
    EXPECT_THROW(parser.parse(), ParseError);
}

// 4. if에서 LEFT_PAREN 누락: `if a > 1) { print 1; }`
TEST(ParserErrorTest, IfMissingLeftParen) {
    std::vector<Token> tokens = {
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
    };
    Parser parser(tokens);
    EXPECT_THROW(parser.parse(), ParseError);
}

// 5. if에서 RIGHT_PAREN 누락: `if (a > 1 { print 1; }`
TEST(ParserErrorTest, IfMissingRightParen) {
    std::vector<Token> tokens = {
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
    };
    Parser parser(tokens);
    EXPECT_THROW(parser.parse(), ParseError);
}

// 6. 블록에서 RIGHT_BRACE 누락: `{ print 1;`
TEST(ParserErrorTest, BlockMissingRightBrace) {
    std::vector<Token> tokens = {
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 1),
        Token(TokenType::PRINT,       "print", nullptr, 1),
        Token(TokenType::NUMBER,      "1",     1.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        // RIGHT_BRACE 누락
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 1),
    };
    Parser parser(tokens);
    EXPECT_THROW(parser.parse(), ParseError);
}