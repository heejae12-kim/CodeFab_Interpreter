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

TEST(ParserTest, MultipleVarAndPrint) {
    std::vector<Token> tokens = {
        // var a = 10;
        Token(TokenType::VAR,        "var",   nullptr, 1),
        Token(TokenType::IDENTIFIER, "a",     nullptr, 1),
        Token(TokenType::EQUAL,      "=",     nullptr, 1),
        Token(TokenType::NUMBER,     "10",    10.0, 1),
        Token(TokenType::SEMICOLON,  ";",     nullptr, 1),
// var b = 3 * 6;
        Token(TokenType::VAR,        "var",   nullptr, 2),
        Token(TokenType::IDENTIFIER, "b",     nullptr, 2),
        Token(TokenType::EQUAL,      "=",     nullptr, 2),
        Token(TokenType::NUMBER,     "3",     3.0, 2),
        Token(TokenType::STAR,       "*",     nullptr, 2),
        Token(TokenType::NUMBER,     "6",     6.0, 2),
        Token(TokenType::SEMICOLON,  ";",     nullptr, 2),
// var c = 5 / 2;
        Token(TokenType::VAR,        "var",   nullptr, 3),
        Token(TokenType::IDENTIFIER, "c",     nullptr, 3),
        Token(TokenType::EQUAL,      "=",     nullptr, 3),
        Token(TokenType::NUMBER,     "5",     5.0, 3),
        Token(TokenType::SLASH,      "/",     nullptr, 3),
        Token(TokenType::NUMBER,     "2",     2.0, 3),
        Token(TokenType::SEMICOLON,  ";",     nullptr, 3),
// print b - c;
        Token(TokenType::PRINT,      "print", nullptr, 4),
        Token(TokenType::IDENTIFIER, "b",     nullptr, 4),
        Token(TokenType::MINUS,      "-",     nullptr, 4),
        Token(TokenType::IDENTIFIER, "c",     nullptr, 4),
        Token(TokenType::SEMICOLON,  ";",     nullptr, 4),
        Token(TokenType::EOF_TOKEN,  "",      nullptr, 4),
    };

    Parser parser(tokens);
    auto stmts = parser.parse();

    ASSERT_EQ(stmts.size(), 4u);

    // var a = 10;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "a");
            auto* lit = dynamic_cast<LiteralExpr*>(stmt.getInitializer().get());
            ASSERT_NE(lit, nullptr);

            EXPECT_DOUBLE_EQ(std::get<double>(lit->getValue()), 10.0);
            });
        stmts[0]->accept(mock);
    }

    // var b = 3 * 6;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "b");
            auto* bin = dynamic_cast<BinaryExpr*>(stmt.getInitializer().get());
            ASSERT_NE(bin, nullptr);
            EXPECT_EQ(bin->getOp().getTokenType(), TokenType::STAR);
            auto* left = dynamic_cast<LiteralExpr*>(bin->getLeft().get());
            auto* right = dynamic_cast<LiteralExpr*>(bin->getRight().get());

            EXPECT_DOUBLE_EQ(std::get<double>(left->getValue()), 3.0);

            EXPECT_DOUBLE_EQ(std::get<double>(right->getValue()), 6.0);
            });
        stmts[1]->accept(mock);
    }

    // var c = 5 / 2;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "c");
            auto* bin = dynamic_cast<BinaryExpr*>(stmt.getInitializer().get());
            ASSERT_NE(bin, nullptr);
            EXPECT_EQ(bin->getOp().getTokenType(), TokenType::SLASH);
            auto* left = dynamic_cast<LiteralExpr*>(bin->getLeft().get());
            auto* right = dynamic_cast<LiteralExpr*>(bin->getRight().get());

            EXPECT_DOUBLE_EQ(std::get<double>(left->getValue()), 5.0);

            EXPECT_DOUBLE_EQ(std::get<double>(right->getValue()), 2.0);
            });
        stmts[2]->accept(mock);
    }

    // print b - c;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitPrintStmt(testing::_))
            .WillOnce([](PrintStmt& stmt) {
            auto* bin = dynamic_cast<BinaryExpr*>(stmt.getExpression().get());
            ASSERT_NE(bin, nullptr);
            EXPECT_EQ(bin->getOp().getTokenType(), TokenType::MINUS);
            auto* left = dynamic_cast<VariableExpr*>(bin->getLeft().get());
            auto* right = dynamic_cast<VariableExpr*>(bin->getRight().get());
            ASSERT_NE(left, nullptr);
            ASSERT_NE(right, nullptr);
            EXPECT_EQ(left->getName().getLexme(), "b");
            EXPECT_EQ(right->getName().getLexme(), "c");
            });
        stmts[3]->accept(mock);
    }
}

TEST(ParserTest, IfStatement) {
    std::vector<Token> tokens = {
        // var a = 5;
        Token(TokenType::VAR,         "var",   nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::EQUAL,       "=",     nullptr, 1),
        Token(TokenType::NUMBER,      "5",     5.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        // if (a > 1) {
        Token(TokenType::IF,          "if",    nullptr, 2),
        Token(TokenType::LEFT_PAREN,  "(",     nullptr, 2),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 2),
        Token(TokenType::GREATER,     ">",     nullptr, 2),
        Token(TokenType::NUMBER,      "1",     1.0,     2),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 2),
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 2),
        // print 3;
        Token(TokenType::PRINT,       "print", nullptr, 3),
        Token(TokenType::NUMBER,      "3",     3.0,     3),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 3),
        // }
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 4),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 4),
    };

    Parser parser(tokens);
    auto stmts = parser.parse();

    ASSERT_EQ(stmts.size(), 2u);

    // var a = 5;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "a");
            auto* lit =
                dynamic_cast<LiteralExpr*>(stmt.getInitializer().get());
            ASSERT_NE(lit, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(lit->getValue()),
                5.0);
                });
        stmts[0]->accept(mock);
    }

    // if (a > 1) { print 3; }
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitIfStmt(testing::_))
            .WillOnce([](IfStmt& stmt) {
            // condition: a > 1
            auto* cond =
                dynamic_cast<BinaryExpr*>(stmt.getCondition().get());
            ASSERT_NE(cond, nullptr);
            EXPECT_EQ(cond->getOp().getTokenType(),
                TokenType::GREATER);

            auto* left =
                dynamic_cast<VariableExpr*>(cond->getLeft().get());
            ASSERT_NE(left, nullptr);
            EXPECT_EQ(left->getName().getLexme(), "a");

            auto* right =
                dynamic_cast<LiteralExpr*>(cond->getRight().get());
            ASSERT_NE(right, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(right->getValue()),
                1.0);

            // thenBranch: { print 3; }
            auto* block =
                dynamic_cast<BlockStmt*>(stmt.getThenBranch().get());
            ASSERT_NE(block, nullptr);
            ASSERT_EQ(block->getStatements().size(), 1u);

            auto* print = dynamic_cast<PrintStmt*>(block->getStatements()[0].get());
            ASSERT_NE(print, nullptr);
            auto* lit = dynamic_cast<LiteralExpr*>(print->getExpression().get());
            ASSERT_NE(lit, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(lit->getValue()), 3.0);

            // elseBranch 없음
            EXPECT_EQ(stmt.getElseBranch(), nullptr);
            });
        stmts[1]->accept(mock);
    }
}

TEST(ParserTest, IfElseStatement) {
    std::vector<Token> tokens = {
        // var a = 5;
        Token(TokenType::VAR,         "var",   nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::EQUAL,       "=",     nullptr, 1),
        Token(TokenType::NUMBER,      "5",     5.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        // if (a < 1) {
        Token(TokenType::IF,          "if",    nullptr, 2),
        Token(TokenType::LEFT_PAREN,  "(",     nullptr, 2),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 2),
        Token(TokenType::LESS,        "<",     nullptr, 2),
        Token(TokenType::NUMBER,      "1",     1.0,     2),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 2),
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 2),
        // print 3;
        Token(TokenType::PRINT,       "print", nullptr, 3),
        Token(TokenType::NUMBER,      "3",     3.0,     3),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 3),
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 4),
        // else {
        Token(TokenType::ELSE,        "else",  nullptr, 5),
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 5),
        // print 5;
        Token(TokenType::PRINT,       "print", nullptr, 6),
        Token(TokenType::NUMBER,      "5",     5.0,     6),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 6),
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 7),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 7),
    };

    Parser parser(tokens);
    auto stmts = parser.parse();

    ASSERT_EQ(stmts.size(), 2u);

    // var a = 5;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "a");
            auto* lit = dynamic_cast<LiteralExpr*>(stmt.getInitializer().get());
            ASSERT_NE(lit, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(lit->getValue()), 5.0);
            });
        stmts[0]->accept(mock);
    }

    // if (a < 1) { print 3; } else { print 5; }
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitIfStmt(testing::_))
            .WillOnce([](IfStmt& stmt) {
            // condition: a < 1
            auto* cond = dynamic_cast<BinaryExpr*>(stmt.getCondition().get());
            ASSERT_NE(cond, nullptr);
            EXPECT_EQ(cond->getOp().getTokenType(), TokenType::LESS);

            auto* left = dynamic_cast<VariableExpr*>(cond->getLeft().get());
            ASSERT_NE(left, nullptr);
            EXPECT_EQ(left->getName().getLexme(), "a");

            auto* right = dynamic_cast<LiteralExpr*>(cond->getRight().get());
            ASSERT_NE(right, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(right->getValue()), 1.0);

            // thenBranch: { print 3; }
            auto* thenBlock = dynamic_cast<BlockStmt*>(stmt.getThenBranch().get());
            ASSERT_NE(thenBlock, nullptr);
            ASSERT_EQ(thenBlock->getStatements().size(), 1u);
            auto* thenPrint = dynamic_cast<PrintStmt*>(thenBlock->getStatements()[0].get());
            ASSERT_NE(thenPrint, nullptr);
            auto* thenLit = dynamic_cast<LiteralExpr*>(thenPrint->getExpression().get());
            ASSERT_NE(thenLit, nullptr);

            EXPECT_DOUBLE_EQ(std::get<double>(thenLit->getValue()), 3.0);

            // elseBranch: { print 5; }
            auto* elseBlock = dynamic_cast<BlockStmt*>(stmt.getElseBranch().get());
            ASSERT_NE(elseBlock, nullptr);
            ASSERT_EQ(elseBlock->getStatements().size(), 1u);
            auto* elsePrint = dynamic_cast<PrintStmt*>(elseBlock->getStatements()[0].get());
            ASSERT_NE(elsePrint, nullptr);
            auto* elseLit = dynamic_cast<LiteralExpr*>(elsePrint->getExpression().get());
            ASSERT_NE(elseLit, nullptr);

            EXPECT_DOUBLE_EQ(std::get<double>(elseLit->getValue()), 5.0);
            });
        stmts[1]->accept(mock);
    }
}
