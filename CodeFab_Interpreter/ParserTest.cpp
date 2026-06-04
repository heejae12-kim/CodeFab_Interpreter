#include "Parser.h"
#include "gmock/gmock.h"

class MockStmtVisitor : public StmtVisitor {
public:
    MOCK_METHOD(void, visitPrintStmt, (PrintStmt&),
        (override));
    MOCK_METHOD(void, visitExprStmt, (ExprStmt&),
        (override));
    MOCK_METHOD(void, visitVarStmt, (VarStmt&),
        (override));
    MOCK_METHOD(void, visitBlockStmt, (BlockStmt&),
        (override));
    MOCK_METHOD(void, visitIfStmt, (IfStmt&),
        (override));
    MOCK_METHOD(void, visitForStmt, (ForStmt&),
        (override));
};

TEST(ParserTest, VarDeclaration) {
    std::vector<Token> tokens = {
        Token(TokenType::VAR,        "var", nullptr, 1),
        Token(TokenType::IDENTIFIER, "a",   nullptr, 1),
        Token(TokenType::EQUAL,      "=",   nullptr, 1),
        Token(TokenType::NUMBER,     "10",  10.0,    1),
        Token(TokenType::SEMICOLON,  ";",   nullptr, 1),
        Token(TokenType::EOF_TOKEN,  "",    nullptr, 1),
    };

    Parser parser(tokens);
    auto stmts = parser.parse();

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitVarStmt(testing::_))
        .WillOnce([](VarStmt& stmt) {
        EXPECT_EQ(stmt.getName().getLexme(), "a");
        EXPECT_EQ(stmt.getName().getTokenType(),
            TokenType::IDENTIFIER);

        auto* lit =
            dynamic_cast<LiteralExpr*>(stmt.getInitializer().get());
        ASSERT_NE(lit, nullptr);

        EXPECT_DOUBLE_EQ(std::get<double>(lit->getValue()),10.0);
            });

    stmts[0]->accept(mock);
}


TEST(ParserTest, PrintAddition) {
    std::vector<Token> tokens = {
        Token(TokenType::PRINT, "print", nullptr, 1),
        Token(TokenType::NUMBER, "1", 1.0, 1),
        Token(TokenType::PLUS, "+", nullptr, 1),
        Token(TokenType::NUMBER, "2", 2.0, 1),
        Token(TokenType::SEMICOLON, ";", nullptr, 1),
        Token(TokenType::EOF_TOKEN, "", nullptr, 1),
    };

    Parser parser(tokens);
    auto stmts = parser.parse();

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitPrintStmt(testing::_))
        .WillOnce([](PrintStmt& stmt) {
        auto* bin = dynamic_cast<BinaryExpr*>(stmt.getExpression().get());
        ASSERT_NE(bin, nullptr);
        EXPECT_EQ(bin->getOp().getTokenType(), TokenType::PLUS);

        auto* left = dynamic_cast<LiteralExpr*>(bin->getLeft().get());
        ASSERT_NE(left, nullptr);

        EXPECT_DOUBLE_EQ(std::get<double>(left->getValue()), 1.0);

        auto* right = dynamic_cast<LiteralExpr*>(bin->getRight().get());
        ASSERT_NE(right, nullptr);

        EXPECT_DOUBLE_EQ(std::get<double>(right->getValue()), 2.0);
        });

    stmts[0]->accept(mock);
}
