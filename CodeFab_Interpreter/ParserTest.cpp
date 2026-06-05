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

class ParserFixture : public testing::Test {
protected:
    std::vector<StmtPtr> parse(std::vector<Token> tokens) {
        Parser parser(std::move(tokens));
        return parser.parse();
    }
};

TEST_F(ParserFixture, VarDeclaration) {
    auto stmts = parse({
        Token(TokenType::VAR,        "var", nullptr, 1),
        Token(TokenType::IDENTIFIER, "a",   nullptr, 1),
        Token(TokenType::EQUAL,      "=",   nullptr, 1),
        Token(TokenType::NUMBER,     "10",  10.0,    1),
        Token(TokenType::SEMICOLON,  ";",   nullptr, 1),
        Token(TokenType::EOF_TOKEN,  "",    nullptr, 1),
    });

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitVarStmt(testing::_))
        .WillOnce([](VarStmt& stmt) {
        EXPECT_EQ(stmt.getName().getLexme(), "a");
        EXPECT_EQ(stmt.getName().getTokenType(), TokenType::IDENTIFIER);
        auto* lit = dynamic_cast<LiteralExpr*>(stmt.getInitializer().get());
        ASSERT_NE(lit, nullptr);
        EXPECT_DOUBLE_EQ(std::get<double>(lit->getValue()), 10.0);
        });
    stmts[0]->accept(mock);
}

TEST_F(ParserFixture, PrintAddition) {
    auto stmts = parse({
        Token(TokenType::PRINT,     "print", nullptr, 1),
        Token(TokenType::NUMBER,    "1",     1.0,     1),
        Token(TokenType::PLUS,      "+",     nullptr, 1),
        Token(TokenType::NUMBER,    "2",     2.0,     1),
        Token(TokenType::SEMICOLON, ";",     nullptr, 1),
        Token(TokenType::EOF_TOKEN, "",      nullptr, 1),
    });

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

TEST_F(ParserFixture, MultipleVarAndPrint) {
    auto stmts = parse({
        // var a = 10;
        Token(TokenType::VAR,        "var",   nullptr, 1),
        Token(TokenType::IDENTIFIER, "a",     nullptr, 1),
        Token(TokenType::EQUAL,      "=",     nullptr, 1),
        Token(TokenType::NUMBER,     "10",    10.0,    1),
        Token(TokenType::SEMICOLON,  ";",     nullptr, 1),
        // var b = 3 * 6;
        Token(TokenType::VAR,        "var",   nullptr, 2),
        Token(TokenType::IDENTIFIER, "b",     nullptr, 2),
        Token(TokenType::EQUAL,      "=",     nullptr, 2),
        Token(TokenType::NUMBER,     "3",     3.0,     2),
        Token(TokenType::STAR,       "*",     nullptr, 2),
        Token(TokenType::NUMBER,     "6",     6.0,     2),
        Token(TokenType::SEMICOLON,  ";",     nullptr, 2),
        // var c = 5 / 2;
        Token(TokenType::VAR,        "var",   nullptr, 3),
        Token(TokenType::IDENTIFIER, "c",     nullptr, 3),
        Token(TokenType::EQUAL,      "=",     nullptr, 3),
        Token(TokenType::NUMBER,     "5",     5.0,     3),
        Token(TokenType::SLASH,      "/",     nullptr, 3),
        Token(TokenType::NUMBER,     "2",     2.0,     3),
        Token(TokenType::SEMICOLON,  ";",     nullptr, 3),
        // print b - c;
        Token(TokenType::PRINT,      "print", nullptr, 4),
        Token(TokenType::IDENTIFIER, "b",     nullptr, 4),
        Token(TokenType::MINUS,      "-",     nullptr, 4),
        Token(TokenType::IDENTIFIER, "c",     nullptr, 4),
        Token(TokenType::SEMICOLON,  ";",     nullptr, 4),
        Token(TokenType::EOF_TOKEN,  "",      nullptr, 4),
    });

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
            auto* left  = dynamic_cast<LiteralExpr*>(bin->getLeft().get());
            auto* right = dynamic_cast<LiteralExpr*>(bin->getRight().get());
            EXPECT_DOUBLE_EQ(std::get<double>(left->getValue()),  3.0);
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
            auto* left  = dynamic_cast<LiteralExpr*>(bin->getLeft().get());
            auto* right = dynamic_cast<LiteralExpr*>(bin->getRight().get());
            EXPECT_DOUBLE_EQ(std::get<double>(left->getValue()),  5.0);
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
            auto* left  = dynamic_cast<VariableExpr*>(bin->getLeft().get());
            auto* right = dynamic_cast<VariableExpr*>(bin->getRight().get());
            ASSERT_NE(left,  nullptr);
            ASSERT_NE(right, nullptr);
            EXPECT_EQ(left->getName().getLexme(),  "b");
            EXPECT_EQ(right->getName().getLexme(), "c");
            });
        stmts[3]->accept(mock);
    }
}

TEST_F(ParserFixture, IfStatement) {
    auto stmts = parse({
        // var a = 5;
        Token(TokenType::VAR,         "var",   nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::EQUAL,       "=",     nullptr, 1),
        Token(TokenType::NUMBER,      "5",     5.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        // if (a > 1) { print 3; }
        Token(TokenType::IF,          "if",    nullptr, 2),
        Token(TokenType::LEFT_PAREN,  "(",     nullptr, 2),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 2),
        Token(TokenType::GREATER,     ">",     nullptr, 2),
        Token(TokenType::NUMBER,      "1",     1.0,     2),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 2),
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 2),
        Token(TokenType::PRINT,       "print", nullptr, 3),
        Token(TokenType::NUMBER,      "3",     3.0,     3),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 3),
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 4),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 4),
    });

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
    // if (a > 1) { print 3; }
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitIfStmt(testing::_))
            .WillOnce([](IfStmt& stmt) {
            auto* cond = dynamic_cast<BinaryExpr*>(stmt.getCondition().get());
            ASSERT_NE(cond, nullptr);
            EXPECT_EQ(cond->getOp().getTokenType(), TokenType::GREATER);
            auto* left = dynamic_cast<VariableExpr*>(cond->getLeft().get());
            ASSERT_NE(left, nullptr);
            EXPECT_EQ(left->getName().getLexme(), "a");
            auto* right = dynamic_cast<LiteralExpr*>(cond->getRight().get());
            ASSERT_NE(right, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(right->getValue()), 1.0);
            auto* block = dynamic_cast<BlockStmt*>(stmt.getThenBranch().get());
            ASSERT_NE(block, nullptr);
            ASSERT_EQ(block->getStatements().size(), 1u);
            auto* print = dynamic_cast<PrintStmt*>(block->getStatements()[0].get());
            ASSERT_NE(print, nullptr);
            auto* lit = dynamic_cast<LiteralExpr*>(print->getExpression().get());
            ASSERT_NE(lit, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(lit->getValue()), 3.0);
            EXPECT_EQ(stmt.getElseBranch(), nullptr);
            });
        stmts[1]->accept(mock);
    }
}

TEST_F(ParserFixture, IfElseStatement) {
    auto stmts = parse({
        // var a = 5;
        Token(TokenType::VAR,         "var",   nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::EQUAL,       "=",     nullptr, 1),
        Token(TokenType::NUMBER,      "5",     5.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        // if (a < 1) { print 3; } else { print 5; }
        Token(TokenType::IF,          "if",    nullptr, 2),
        Token(TokenType::LEFT_PAREN,  "(",     nullptr, 2),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 2),
        Token(TokenType::LESS,        "<",     nullptr, 2),
        Token(TokenType::NUMBER,      "1",     1.0,     2),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 2),
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 2),
        Token(TokenType::PRINT,       "print", nullptr, 3),
        Token(TokenType::NUMBER,      "3",     3.0,     3),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 3),
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 4),
        Token(TokenType::ELSE,        "else",  nullptr, 5),
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 5),
        Token(TokenType::PRINT,       "print", nullptr, 6),
        Token(TokenType::NUMBER,      "5",     5.0,     6),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 6),
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 7),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 7),
    });

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
            auto* cond = dynamic_cast<BinaryExpr*>(stmt.getCondition().get());
            ASSERT_NE(cond, nullptr);
            EXPECT_EQ(cond->getOp().getTokenType(), TokenType::LESS);
            auto* left = dynamic_cast<VariableExpr*>(cond->getLeft().get());
            ASSERT_NE(left, nullptr);
            EXPECT_EQ(left->getName().getLexme(), "a");
            auto* right = dynamic_cast<LiteralExpr*>(cond->getRight().get());
            ASSERT_NE(right, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(right->getValue()), 1.0);

            auto* thenBlock = dynamic_cast<BlockStmt*>(stmt.getThenBranch().get());
            ASSERT_NE(thenBlock, nullptr);
            ASSERT_EQ(thenBlock->getStatements().size(), 1u);
            auto* thenPrint = dynamic_cast<PrintStmt*>(thenBlock->getStatements()[0].get());
            ASSERT_NE(thenPrint, nullptr);
            auto* thenLit = dynamic_cast<LiteralExpr*>(thenPrint->getExpression().get());
            ASSERT_NE(thenLit, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(thenLit->getValue()), 3.0);

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

TEST_F(ParserFixture, ForStatement) {
    auto stmts = parse({
        // var a;
        Token(TokenType::VAR,         "var",   nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        // for (a = 0; a < 4; a = a + 1) { print a; }
        Token(TokenType::FOR,         "for",   nullptr, 2),
        Token(TokenType::LEFT_PAREN,  "(",     nullptr, 2),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 2),
        Token(TokenType::EQUAL,       "=",     nullptr, 2),
        Token(TokenType::NUMBER,      "0",     0.0,     2),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 2),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 2),
        Token(TokenType::LESS,        "<",     nullptr, 2),
        Token(TokenType::NUMBER,      "4",     4.0,     2),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 2),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 2),
        Token(TokenType::EQUAL,       "=",     nullptr, 2),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 2),
        Token(TokenType::PLUS,        "+",     nullptr, 2),
        Token(TokenType::NUMBER,      "1",     1.0,     2),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 2),
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 2),
        Token(TokenType::PRINT,       "print", nullptr, 3),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 3),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 3),
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 3),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 3),
    });

    ASSERT_EQ(stmts.size(), 2u);

    // var a; (초기화 없음)
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "a");
            EXPECT_EQ(stmt.getInitializer(), nullptr);
            });
        stmts[0]->accept(mock);
    }
    // for (a = 0; a < 4; a = a + 1) { print a; }
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitForStmt(testing::_))
            .WillOnce([](ForStmt& stmt) {
            auto* initStmt = dynamic_cast<ExprStmt*>(stmt.getInitializer().get());
            ASSERT_NE(initStmt, nullptr);
            auto* assign = dynamic_cast<AssignExpr*>(initStmt->getExpression().get());
            ASSERT_NE(assign, nullptr);
            EXPECT_EQ(assign->getName().getLexme(), "a");
            auto* initVal = dynamic_cast<LiteralExpr*>(assign->getValue().get());
            ASSERT_NE(initVal, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(initVal->getValue()), 0.0);

            auto* cond = dynamic_cast<BinaryExpr*>(stmt.getCondition().get());
            ASSERT_NE(cond, nullptr);
            EXPECT_EQ(cond->getOp().getTokenType(), TokenType::LESS);
            auto* condLeft = dynamic_cast<VariableExpr*>(cond->getLeft().get());
            ASSERT_NE(condLeft, nullptr);
            EXPECT_EQ(condLeft->getName().getLexme(), "a");
            auto* condRight = dynamic_cast<LiteralExpr*>(cond->getRight().get());
            ASSERT_NE(condRight, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(condRight->getValue()), 4.0);

            auto* incr = dynamic_cast<AssignExpr*>(stmt.getIncrement().get());
            ASSERT_NE(incr, nullptr);
            EXPECT_EQ(incr->getName().getLexme(), "a");
            auto* incrBin = dynamic_cast<BinaryExpr*>(incr->getValue().get());
            ASSERT_NE(incrBin, nullptr);
            EXPECT_EQ(incrBin->getOp().getTokenType(), TokenType::PLUS);
            auto* incrLeft = dynamic_cast<VariableExpr*>(incrBin->getLeft().get());
            ASSERT_NE(incrLeft, nullptr);
            EXPECT_EQ(incrLeft->getName().getLexme(), "a");
            auto* incrRight = dynamic_cast<LiteralExpr*>(incrBin->getRight().get());
            ASSERT_NE(incrRight, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(incrRight->getValue()), 1.0);

            auto* body = dynamic_cast<BlockStmt*>(stmt.getBody().get());
            ASSERT_NE(body, nullptr);
            ASSERT_EQ(body->getStatements().size(), 1u);
            auto* print = dynamic_cast<PrintStmt*>(body->getStatements()[0].get());
            ASSERT_NE(print, nullptr);
            auto* printVar = dynamic_cast<VariableExpr*>(print->getExpression().get());
            ASSERT_NE(printVar, nullptr);
            EXPECT_EQ(printVar->getName().getLexme(), "a");
            });
        stmts[1]->accept(mock);
    }
}

TEST_F(ParserFixture, StringVarAndPrint) {
    auto stmts = parse({
        // var str = "abdc";
        Token(TokenType::VAR,        "var",  nullptr,             1),
        Token(TokenType::IDENTIFIER, "str",  nullptr,             1),
        Token(TokenType::EQUAL,      "=",    nullptr,             1),
        Token(TokenType::STRING,     "abdc", std::string("abdc"), 1),
        Token(TokenType::SEMICOLON,  ";",    nullptr,             1),
        // print str;
        Token(TokenType::PRINT,      "print", nullptr, 2),
        Token(TokenType::IDENTIFIER, "str",   nullptr, 2),
        Token(TokenType::SEMICOLON,  ";",     nullptr, 2),
        Token(TokenType::EOF_TOKEN,  "",      nullptr, 2),
    });

    ASSERT_EQ(stmts.size(), 2u);

    // var str = "abdc";
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "str");
            auto* lit = dynamic_cast<LiteralExpr*>(stmt.getInitializer().get());
            ASSERT_NE(lit, nullptr);
            ASSERT_TRUE(std::holds_alternative<std::string>(lit->getValue()));
            EXPECT_EQ(std::get<std::string>(lit->getValue()), "abdc");
            });
        stmts[0]->accept(mock);
    }
    // print str;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitPrintStmt(testing::_))
            .WillOnce([](PrintStmt& stmt) {
            auto* var = dynamic_cast<VariableExpr*>(stmt.getExpression().get());
            ASSERT_NE(var, nullptr);
            EXPECT_EQ(var->getName().getLexme(), "str");
            });
        stmts[1]->accept(mock);
    }
}

TEST_F(ParserFixture, BoolVarAndIfPrint) {
    auto stmts = parse({
        // var test_bool1 = true;
        Token(TokenType::VAR,         "var",        nullptr,             1),
        Token(TokenType::IDENTIFIER,  "test_bool1", nullptr,             1),
        Token(TokenType::EQUAL,       "=",          nullptr,             1),
        Token(TokenType::TRUE_KW,     "true",       true,                1),
        Token(TokenType::SEMICOLON,   ";",          nullptr,             1),
        // if (test_bool1) print "true";
        Token(TokenType::IF,          "if",         nullptr,             2),
        Token(TokenType::LEFT_PAREN,  "(",          nullptr,             2),
        Token(TokenType::IDENTIFIER,  "test_bool1", nullptr,             2),
        Token(TokenType::RIGHT_PAREN, ")",          nullptr,             2),
        Token(TokenType::PRINT,       "print",      nullptr,             2),
        Token(TokenType::STRING,      "true",       std::string("true"), 2),
        Token(TokenType::SEMICOLON,   ";",          nullptr,             2),
        Token(TokenType::EOF_TOKEN,   "",           nullptr,             2),
    });

    ASSERT_EQ(stmts.size(), 2u);

    // var test_bool1 = true;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "test_bool1");
            auto* lit = dynamic_cast<LiteralExpr*>(stmt.getInitializer().get());
            ASSERT_NE(lit, nullptr);
            ASSERT_TRUE(std::holds_alternative<bool>(lit->getValue()));
            EXPECT_EQ(std::get<bool>(lit->getValue()), true);
            });
        stmts[0]->accept(mock);
    }
    // if (test_bool1) print "true";
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitIfStmt(testing::_))
            .WillOnce([](IfStmt& stmt) {
            auto* cond = dynamic_cast<VariableExpr*>(stmt.getCondition().get());
            ASSERT_NE(cond, nullptr);
            EXPECT_EQ(cond->getName().getLexme(), "test_bool1");
            auto* print = dynamic_cast<PrintStmt*>(stmt.getThenBranch().get());
            ASSERT_NE(print, nullptr);
            auto* lit = dynamic_cast<LiteralExpr*>(print->getExpression().get());
            ASSERT_NE(lit, nullptr);
            ASSERT_TRUE(std::holds_alternative<std::string>(lit->getValue()));
            EXPECT_EQ(std::get<std::string>(lit->getValue()), "true");
            EXPECT_EQ(stmt.getElseBranch(), nullptr);
            });
        stmts[1]->accept(mock);
    }
}

TEST_F(ParserFixture, BoolEqualityAndPrint) {
    auto stmts = parse({
        // var test_bool = false;
        Token(TokenType::VAR,         "var",       nullptr,              1),
        Token(TokenType::IDENTIFIER,  "test_bool", nullptr,              1),
        Token(TokenType::EQUAL,       "=",         nullptr,              1),
        Token(TokenType::FALSE_KW,    "false",     false,                1),
        Token(TokenType::SEMICOLON,   ";",         nullptr,              1),
        // if (test_bool == false) print "false";
        Token(TokenType::IF,          "if",        nullptr,              2),
        Token(TokenType::LEFT_PAREN,  "(",         nullptr,              2),
        Token(TokenType::IDENTIFIER,  "test_bool", nullptr,              2),
        Token(TokenType::EQUAL_EQUAL, "==",        nullptr,              2),
        Token(TokenType::FALSE_KW,    "false",     false,                2),
        Token(TokenType::RIGHT_PAREN, ")",         nullptr,              2),
        Token(TokenType::PRINT,       "print",     nullptr,              2),
        Token(TokenType::STRING,      "false",     std::string("false"), 2),
        Token(TokenType::SEMICOLON,   ";",         nullptr,              2),
        Token(TokenType::EOF_TOKEN,   "",          nullptr,              2),
    });

    ASSERT_EQ(stmts.size(), 2u);

    // var test_bool = false;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "test_bool");
            auto* lit = dynamic_cast<LiteralExpr*>(stmt.getInitializer().get());
            ASSERT_NE(lit, nullptr);
            ASSERT_TRUE(std::holds_alternative<bool>(lit->getValue()));
            EXPECT_EQ(std::get<bool>(lit->getValue()), false);
            });
        stmts[0]->accept(mock);
    }
    // if (test_bool == false) print "false";
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitIfStmt(testing::_))
            .WillOnce([](IfStmt& stmt) {
            auto* cond = dynamic_cast<BinaryExpr*>(stmt.getCondition().get());
            ASSERT_NE(cond, nullptr);
            EXPECT_EQ(cond->getOp().getTokenType(), TokenType::EQUAL_EQUAL);
            auto* left = dynamic_cast<VariableExpr*>(cond->getLeft().get());
            ASSERT_NE(left, nullptr);
            EXPECT_EQ(left->getName().getLexme(), "test_bool");
            auto* right = dynamic_cast<LiteralExpr*>(cond->getRight().get());
            ASSERT_NE(right, nullptr);
            ASSERT_TRUE(std::holds_alternative<bool>(right->getValue()));
            EXPECT_EQ(std::get<bool>(right->getValue()), false);
            auto* print = dynamic_cast<PrintStmt*>(stmt.getThenBranch().get());
            ASSERT_NE(print, nullptr);
            auto* lit = dynamic_cast<LiteralExpr*>(print->getExpression().get());
            ASSERT_NE(lit, nullptr);
            ASSERT_TRUE(std::holds_alternative<std::string>(lit->getValue()));
            EXPECT_EQ(std::get<std::string>(lit->getValue()), "false");
            EXPECT_EQ(stmt.getElseBranch(), nullptr);
            });
        stmts[1]->accept(mock);
    }
}

TEST_F(ParserFixture, OrOperator) {
    auto stmts = parse({
        // var a = true or false;
        Token(TokenType::VAR,        "var",   nullptr, 1),
        Token(TokenType::IDENTIFIER, "a",     nullptr, 1),
        Token(TokenType::EQUAL,      "=",     nullptr, 1),
        Token(TokenType::TRUE_KW,    "true",  true,    1),
        Token(TokenType::OR_OP,      "or",    nullptr, 1),
        Token(TokenType::FALSE_KW,   "false", false,   1),
        Token(TokenType::SEMICOLON,  ";",     nullptr, 1),
        Token(TokenType::EOF_TOKEN,  "",      nullptr, 1),
    });

    ASSERT_EQ(stmts.size(), 1u);

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitVarStmt(testing::_))
        .WillOnce([](VarStmt& stmt) {
        EXPECT_EQ(stmt.getName().getLexme(), "a");
        auto* bin = dynamic_cast<BinaryExpr*>(stmt.getInitializer().get());
        ASSERT_NE(bin, nullptr);
        EXPECT_EQ(bin->getOp().getTokenType(), TokenType::OR_OP);
        auto* left = dynamic_cast<LiteralExpr*>(bin->getLeft().get());
        ASSERT_NE(left, nullptr);
        ASSERT_TRUE(std::holds_alternative<bool>(left->getValue()));
        EXPECT_EQ(std::get<bool>(left->getValue()), true);
        auto* right = dynamic_cast<LiteralExpr*>(bin->getRight().get());
        ASSERT_NE(right, nullptr);
        ASSERT_TRUE(std::holds_alternative<bool>(right->getValue()));
        EXPECT_EQ(std::get<bool>(right->getValue()), false);
        });
    stmts[0]->accept(mock);
}

TEST_F(ParserFixture, AndOperator) {
    auto stmts = parse({
        // var a = true and false;
        Token(TokenType::VAR,        "var",   nullptr, 1),
        Token(TokenType::IDENTIFIER, "a",     nullptr, 1),
        Token(TokenType::EQUAL,      "=",     nullptr, 1),
        Token(TokenType::TRUE_KW,    "true",  true,    1),
        Token(TokenType::AND_OP,     "and",   nullptr, 1),
        Token(TokenType::FALSE_KW,   "false", false,   1),
        Token(TokenType::SEMICOLON,  ";",     nullptr, 1),
        Token(TokenType::EOF_TOKEN,  "",      nullptr, 1),
    });

    ASSERT_EQ(stmts.size(), 1u);

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitVarStmt(testing::_))
        .WillOnce([](VarStmt& stmt) {
        EXPECT_EQ(stmt.getName().getLexme(), "a");
        auto* bin = dynamic_cast<BinaryExpr*>(stmt.getInitializer().get());
        ASSERT_NE(bin, nullptr);
        EXPECT_EQ(bin->getOp().getTokenType(), TokenType::AND_OP);
        auto* left = dynamic_cast<LiteralExpr*>(bin->getLeft().get());
        ASSERT_NE(left, nullptr);
        ASSERT_TRUE(std::holds_alternative<bool>(left->getValue()));
        EXPECT_EQ(std::get<bool>(left->getValue()), true);
        auto* right = dynamic_cast<LiteralExpr*>(bin->getRight().get());
        ASSERT_NE(right, nullptr);
        ASSERT_TRUE(std::holds_alternative<bool>(right->getValue()));
        EXPECT_EQ(std::get<bool>(right->getValue()), false);
        });
    stmts[0]->accept(mock);
}

TEST_F(ParserFixture, UnaryBang) {
    auto stmts = parse({
        // var a = !true;
        Token(TokenType::VAR,        "var",  nullptr, 1),
        Token(TokenType::IDENTIFIER, "a",    nullptr, 1),
        Token(TokenType::EQUAL,      "=",    nullptr, 1),
        Token(TokenType::BANG,       "!",    nullptr, 1),
        Token(TokenType::TRUE_KW,    "true", true,    1),
        Token(TokenType::SEMICOLON,  ";",    nullptr, 1),
        Token(TokenType::EOF_TOKEN,  "",     nullptr, 1),
    });

    ASSERT_EQ(stmts.size(), 1u);

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitVarStmt(testing::_))
        .WillOnce([](VarStmt& stmt) {
        EXPECT_EQ(stmt.getName().getLexme(), "a");
        auto* unary = dynamic_cast<UnaryExpr*>(stmt.getInitializer().get());
        ASSERT_NE(unary, nullptr);
        EXPECT_EQ(unary->getOp().getTokenType(), TokenType::BANG);
        auto* lit = dynamic_cast<LiteralExpr*>(unary->getRight().get());
        ASSERT_NE(lit, nullptr);
        ASSERT_TRUE(std::holds_alternative<bool>(lit->getValue()));
        EXPECT_EQ(std::get<bool>(lit->getValue()), true);
        });
    stmts[0]->accept(mock);
}

TEST_F(ParserFixture, UnaryBangVariable) {
    auto stmts = parse({
        // var a = true;
        Token(TokenType::VAR,        "var",  nullptr, 1),
        Token(TokenType::IDENTIFIER, "a",    nullptr, 1),
        Token(TokenType::EQUAL,      "=",    nullptr, 1),
        Token(TokenType::TRUE_KW,    "true", true,    1),
        Token(TokenType::SEMICOLON,  ";",    nullptr, 1),
        // var b = !a;
        Token(TokenType::VAR,        "var",  nullptr, 2),
        Token(TokenType::IDENTIFIER, "b",    nullptr, 2),
        Token(TokenType::EQUAL,      "=",    nullptr, 2),
        Token(TokenType::BANG,       "!",    nullptr, 2),
        Token(TokenType::IDENTIFIER, "a",    nullptr, 2),
        Token(TokenType::SEMICOLON,  ";",    nullptr, 2),
        Token(TokenType::EOF_TOKEN,  "",     nullptr, 2),
    });

    ASSERT_EQ(stmts.size(), 2u);

    // var a = true;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "a");
            auto* lit = dynamic_cast<LiteralExpr*>(stmt.getInitializer().get());
            ASSERT_NE(lit, nullptr);
            ASSERT_TRUE(std::holds_alternative<bool>(lit->getValue()));
            EXPECT_EQ(std::get<bool>(lit->getValue()), true);
            });
        stmts[0]->accept(mock);
    }
    // var b = !a;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "b");
            auto* unary = dynamic_cast<UnaryExpr*>(stmt.getInitializer().get());
            ASSERT_NE(unary, nullptr);
            EXPECT_EQ(unary->getOp().getTokenType(), TokenType::BANG);
            auto* var = dynamic_cast<VariableExpr*>(unary->getRight().get());
            ASSERT_NE(var, nullptr);
            EXPECT_EQ(var->getName().getLexme(), "a");
            });
        stmts[1]->accept(mock);
    }
}

TEST_F(ParserFixture, BinaryMinusWithVariable) {
    auto stmts = parse({
        // var a = 3;
        Token(TokenType::VAR,        "var", nullptr, 1),
        Token(TokenType::IDENTIFIER, "a",   nullptr, 1),
        Token(TokenType::EQUAL,      "=",   nullptr, 1),
        Token(TokenType::NUMBER,     "3",   3.0,     1),
        Token(TokenType::SEMICOLON,  ";",   nullptr, 1),
        // var b = 3 - a;
        Token(TokenType::VAR,        "var", nullptr, 2),
        Token(TokenType::IDENTIFIER, "b",   nullptr, 2),
        Token(TokenType::EQUAL,      "=",   nullptr, 2),
        Token(TokenType::NUMBER,     "3",   3.0,     2),
        Token(TokenType::MINUS,      "-",   nullptr, 2),
        Token(TokenType::IDENTIFIER, "a",   nullptr, 2),
        Token(TokenType::SEMICOLON,  ";",   nullptr, 2),
        Token(TokenType::EOF_TOKEN,  "",    nullptr, 2),
    });

    ASSERT_EQ(stmts.size(), 2u);

    // var a = 3;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "a");
            auto* lit = dynamic_cast<LiteralExpr*>(stmt.getInitializer().get());
            ASSERT_NE(lit, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(lit->getValue()), 3.0);
            });
        stmts[0]->accept(mock);
    }
    // var b = 3 - a;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "b");
            auto* bin = dynamic_cast<BinaryExpr*>(stmt.getInitializer().get());
            ASSERT_NE(bin, nullptr);
            EXPECT_EQ(bin->getOp().getTokenType(), TokenType::MINUS);
            auto* left = dynamic_cast<LiteralExpr*>(bin->getLeft().get());
            ASSERT_NE(left, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(left->getValue()), 3.0);
            auto* right = dynamic_cast<VariableExpr*>(bin->getRight().get());
            ASSERT_NE(right, nullptr);
            EXPECT_EQ(right->getName().getLexme(), "a");
            });
        stmts[1]->accept(mock);
    }
}

TEST_F(ParserFixture, UnaryMinus) {
    auto stmts = parse({
        // var a = -3;
        Token(TokenType::VAR,        "var", nullptr, 1),
        Token(TokenType::IDENTIFIER, "a",   nullptr, 1),
        Token(TokenType::EQUAL,      "=",   nullptr, 1),
        Token(TokenType::MINUS,      "-",   nullptr, 1),
        Token(TokenType::NUMBER,     "3",   3.0,     1),
        Token(TokenType::SEMICOLON,  ";",   nullptr, 1),
        Token(TokenType::EOF_TOKEN,  "",    nullptr, 1),
    });

    ASSERT_EQ(stmts.size(), 1u);

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitVarStmt(testing::_))
        .WillOnce([](VarStmt& stmt) {
        EXPECT_EQ(stmt.getName().getLexme(), "a");
        auto* unary = dynamic_cast<UnaryExpr*>(stmt.getInitializer().get());
        ASSERT_NE(unary, nullptr);
        EXPECT_EQ(unary->getOp().getTokenType(), TokenType::MINUS);
        auto* lit = dynamic_cast<LiteralExpr*>(unary->getRight().get());
        ASSERT_NE(lit, nullptr);
        EXPECT_DOUBLE_EQ(std::get<double>(lit->getValue()), 3.0);
        });
    stmts[0]->accept(mock);
}

TEST_F(ParserFixture, FuncDeclarationAndCall) {
    auto stmts = parse({
        // Func test_function() {
        Token(TokenType::FUNC,        "Func",          nullptr,              1),
        Token(TokenType::IDENTIFIER,  "test_function", nullptr,              1),
        Token(TokenType::LEFT_PAREN,  "(",             nullptr,              1),
        Token(TokenType::RIGHT_PAREN, ")",             nullptr,              1),
        Token(TokenType::LEFT_BRACE,  "{",             nullptr,              2),
        // print "hello";
        Token(TokenType::PRINT,       "print",         nullptr,              3),
        Token(TokenType::STRING,      "hello",         std::string("hello"), 3),
        Token(TokenType::SEMICOLON,   ";",             nullptr,              3),
        // return;
        Token(TokenType::RETURN,      "return",        nullptr,              4),
        Token(TokenType::SEMICOLON,   ";",             nullptr,              4),
        // }
        Token(TokenType::RIGHT_BRACE, "}",             nullptr,              5),
        // test_function();
        Token(TokenType::IDENTIFIER,  "test_function", nullptr,              6),
        Token(TokenType::LEFT_PAREN,  "(",             nullptr,              6),
        Token(TokenType::RIGHT_PAREN, ")",             nullptr,              6),
        Token(TokenType::SEMICOLON,   ";",             nullptr,              6),
        // print "end";
        Token(TokenType::PRINT,       "print",         nullptr,              7),
        Token(TokenType::STRING,      "end",           std::string("end"),   7),
        Token(TokenType::SEMICOLON,   ";",             nullptr,              7),
        Token(TokenType::EOF_TOKEN,   "",              nullptr,              7),
    });

    ASSERT_EQ(stmts.size(), 3u);

    // Func test_function() { print "hello"; return; }
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitFuncStmt(testing::_))
            .WillOnce([](FuncStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "test_function");
            EXPECT_TRUE(stmt.getParams().empty());
            ASSERT_EQ(stmt.getBody().size(), 2u);
            auto* print = dynamic_cast<PrintStmt*>(stmt.getBody()[0].get());
            ASSERT_NE(print, nullptr);
            auto* lit = dynamic_cast<LiteralExpr*>(print->getExpression().get());
            ASSERT_NE(lit, nullptr);
            ASSERT_TRUE(std::holds_alternative<std::string>(lit->getValue()));
            EXPECT_EQ(std::get<std::string>(lit->getValue()), "hello");
            auto* ret = dynamic_cast<ReturnStmt*>(stmt.getBody()[1].get());
            ASSERT_NE(ret, nullptr);
            EXPECT_EQ(ret->getValue().get(), nullptr);
            });
        stmts[0]->accept(mock);
    }
    // test_function();
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitExprStmt(testing::_))
            .WillOnce([](ExprStmt& stmt) {
            auto* call = dynamic_cast<CallExpr*>(stmt.getExpression().get());
            ASSERT_NE(call, nullptr);
            auto* callee = dynamic_cast<VariableExpr*>(call->getCallee().get());
            ASSERT_NE(callee, nullptr);
            EXPECT_EQ(callee->getName().getLexme(), "test_function");
            EXPECT_TRUE(call->getArguments().empty());
            });
        stmts[1]->accept(mock);
    }
    // print "end";
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitPrintStmt(testing::_))
            .WillOnce([](PrintStmt& stmt) {
            auto* lit = dynamic_cast<LiteralExpr*>(stmt.getExpression().get());
            ASSERT_NE(lit, nullptr);
            ASSERT_TRUE(std::holds_alternative<std::string>(lit->getValue()));
            EXPECT_EQ(std::get<std::string>(lit->getValue()), "end");
            });
        stmts[2]->accept(mock);
    }
}

TEST_F(ParserFixture, FuncWithParamsAndCall) {
    auto stmts = parse({
        // Func add(a, b) {
        Token(TokenType::FUNC,        "Func",   nullptr, 1),
        Token(TokenType::IDENTIFIER,  "add",    nullptr, 1),
        Token(TokenType::LEFT_PAREN,  "(",      nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",      nullptr, 1),
        Token(TokenType::COMMA,       ",",      nullptr, 1),
        Token(TokenType::IDENTIFIER,  "b",      nullptr, 1),
        Token(TokenType::RIGHT_PAREN, ")",      nullptr, 1),
        Token(TokenType::LEFT_BRACE,  "{",      nullptr, 2),
        // return a + b;
        Token(TokenType::RETURN,      "return", nullptr, 3),
        Token(TokenType::IDENTIFIER,  "a",      nullptr, 3),
        Token(TokenType::PLUS,        "+",      nullptr, 3),
        Token(TokenType::IDENTIFIER,  "b",      nullptr, 3),
        Token(TokenType::SEMICOLON,   ";",      nullptr, 3),
        // }
        Token(TokenType::RIGHT_BRACE, "}",      nullptr, 4),
        // var x = 3;
        Token(TokenType::VAR,         "var",    nullptr, 5),
        Token(TokenType::IDENTIFIER,  "x",      nullptr, 5),
        Token(TokenType::EQUAL,       "=",      nullptr, 5),
        Token(TokenType::NUMBER,      "3",      3.0,     5),
        Token(TokenType::SEMICOLON,   ";",      nullptr, 5),
        // var y = 4;
        Token(TokenType::VAR,         "var",    nullptr, 6),
        Token(TokenType::IDENTIFIER,  "y",      nullptr, 6),
        Token(TokenType::EQUAL,       "=",      nullptr, 6),
        Token(TokenType::NUMBER,      "4",      4.0,     6),
        Token(TokenType::SEMICOLON,   ";",      nullptr, 6),
        // var z = add(x, y);
        Token(TokenType::VAR,         "var",    nullptr, 7),
        Token(TokenType::IDENTIFIER,  "z",      nullptr, 7),
        Token(TokenType::EQUAL,       "=",      nullptr, 7),
        Token(TokenType::IDENTIFIER,  "add",    nullptr, 7),
        Token(TokenType::LEFT_PAREN,  "(",      nullptr, 7),
        Token(TokenType::IDENTIFIER,  "x",      nullptr, 7),
        Token(TokenType::COMMA,       ",",      nullptr, 7),
        Token(TokenType::IDENTIFIER,  "y",      nullptr, 7),
        Token(TokenType::RIGHT_PAREN, ")",      nullptr, 7),
        Token(TokenType::SEMICOLON,   ";",      nullptr, 7),
        // print z;
        Token(TokenType::PRINT,       "print",  nullptr, 8),
        Token(TokenType::IDENTIFIER,  "z",      nullptr, 8),
        Token(TokenType::SEMICOLON,   ";",      nullptr, 8),
        Token(TokenType::EOF_TOKEN,   "",       nullptr, 8),
    });

    ASSERT_EQ(stmts.size(), 5u);

    // Func add(a, b) { return a + b; }
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitFuncStmt(testing::_))
            .WillOnce([](FuncStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "add");
            ASSERT_EQ(stmt.getParams().size(), 2u);
            EXPECT_EQ(stmt.getParams()[0].getLexme(), "a");
            EXPECT_EQ(stmt.getParams()[1].getLexme(), "b");
            ASSERT_EQ(stmt.getBody().size(), 1u);
            auto* ret = dynamic_cast<ReturnStmt*>(stmt.getBody()[0].get());
            ASSERT_NE(ret, nullptr);
            auto* bin = dynamic_cast<BinaryExpr*>(ret->getValue().get());
            ASSERT_NE(bin, nullptr);
            EXPECT_EQ(bin->getOp().getTokenType(), TokenType::PLUS);
            auto* left = dynamic_cast<VariableExpr*>(bin->getLeft().get());
            ASSERT_NE(left, nullptr);
            EXPECT_EQ(left->getName().getLexme(), "a");
            auto* right = dynamic_cast<VariableExpr*>(bin->getRight().get());
            ASSERT_NE(right, nullptr);
            EXPECT_EQ(right->getName().getLexme(), "b");
            });
        stmts[0]->accept(mock);
    }
    // var x = 3;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "x");
            auto* lit = dynamic_cast<LiteralExpr*>(stmt.getInitializer().get());
            ASSERT_NE(lit, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(lit->getValue()), 3.0);
            });
        stmts[1]->accept(mock);
    }
    // var y = 4;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "y");
            auto* lit = dynamic_cast<LiteralExpr*>(stmt.getInitializer().get());
            ASSERT_NE(lit, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(lit->getValue()), 4.0);
            });
        stmts[2]->accept(mock);
    }
    // var z = add(x, y);
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "z");
            auto* call = dynamic_cast<CallExpr*>(stmt.getInitializer().get());
            ASSERT_NE(call, nullptr);
            EXPECT_EQ(dynamic_cast<VariableExpr*>(call->getCallee().get())->getName().getLexme(), "add");
            ASSERT_EQ(call->getArguments().size(), 2u);
            auto* argX = dynamic_cast<VariableExpr*>(call->getArguments()[0].get());
            ASSERT_NE(argX, nullptr);
            EXPECT_EQ(argX->getName().getLexme(), "x");
            auto* argY = dynamic_cast<VariableExpr*>(call->getArguments()[1].get());
            ASSERT_NE(argY, nullptr);
            EXPECT_EQ(argY->getName().getLexme(), "y");
            });
        stmts[3]->accept(mock);
    }
    // print z;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitPrintStmt(testing::_))
            .WillOnce([](PrintStmt& stmt) {
            auto* var = dynamic_cast<VariableExpr*>(stmt.getExpression().get());
            ASSERT_NE(var, nullptr);
            EXPECT_EQ(var->getName().getLexme(), "z");
            });
        stmts[4]->accept(mock);
    }
}

TEST_F(ParserFixture, ArrayCreationAndRead) {
    auto stmts = parse({
        // var arr = Array(3);
        Token(TokenType::VAR,         "var",   nullptr, 1),
        Token(TokenType::IDENTIFIER,  "arr",   nullptr, 1),
        Token(TokenType::EQUAL,       "=",     nullptr, 1),
        Token(TokenType::ARRAY,       "Array", nullptr, 1),
        Token(TokenType::LEFT_PAREN,  "(",     nullptr, 1),
        Token(TokenType::NUMBER,      "3",     3.0,     1),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 1),
    });

    ASSERT_EQ(stmts.size(), 1u);

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitVarStmt(testing::_))
        .WillOnce([](VarStmt& stmt) {
        EXPECT_EQ(stmt.getName().getLexme(), "arr");
        auto* call = dynamic_cast<CallExpr*>(stmt.getInitializer().get());
        ASSERT_NE(call, nullptr);
        auto* callee = dynamic_cast<VariableExpr*>(call->getCallee().get());
        ASSERT_NE(callee, nullptr);
        EXPECT_EQ(callee->getName().getLexme(), "Array");
        ASSERT_EQ(call->getArguments().size(), 1u);
        auto* size = dynamic_cast<LiteralExpr*>(call->getArguments()[0].get());
        ASSERT_NE(size, nullptr);
        EXPECT_DOUBLE_EQ(std::get<double>(size->getValue()), 3.0);
        });
    stmts[0]->accept(mock);
}

TEST_F(ParserFixture, ArrayIndexAssignment) {
    auto stmts = parse({
        // var arr = Array(3);
        Token(TokenType::VAR,          "var",   nullptr, 1),
        Token(TokenType::IDENTIFIER,   "arr",   nullptr, 1),
        Token(TokenType::EQUAL,        "=",     nullptr, 1),
        Token(TokenType::ARRAY,        "Array", nullptr, 1),
        Token(TokenType::LEFT_PAREN,   "(",     nullptr, 1),
        Token(TokenType::NUMBER,       "3",     3.0,     1),
        Token(TokenType::RIGHT_PAREN,  ")",     nullptr, 1),
        Token(TokenType::SEMICOLON,    ";",     nullptr, 1),
        // arr[1] = 5;
        Token(TokenType::IDENTIFIER,   "arr",   nullptr, 2),
        Token(TokenType::LEFT_BRACKET, "[",     nullptr, 2),
        Token(TokenType::NUMBER,       "1",     1.0,     2),
        Token(TokenType::RIGHT_BRACKET,"]",     nullptr, 2),
        Token(TokenType::EQUAL,        "=",     nullptr, 2),
        Token(TokenType::NUMBER,       "5",     5.0,     2),
        Token(TokenType::SEMICOLON,    ";",     nullptr, 2),
        Token(TokenType::EOF_TOKEN,    "",      nullptr, 2),
    });

    ASSERT_EQ(stmts.size(), 2u);

    // var arr = Array(3);
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "arr");
            auto* call = dynamic_cast<CallExpr*>(stmt.getInitializer().get());
            ASSERT_NE(call, nullptr);
            EXPECT_EQ(dynamic_cast<VariableExpr*>(call->getCallee().get())->getName().getLexme(), "Array");
            });
        stmts[0]->accept(mock);
    }
    // arr[1] = 5;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitExprStmt(testing::_))
            .WillOnce([](ExprStmt& stmt) {
            auto* set = dynamic_cast<ArrIndexSetExpr*>(stmt.getExpression().get());
            ASSERT_NE(set, nullptr);
            auto* obj = dynamic_cast<VariableExpr*>(set->getObject().get());
            ASSERT_NE(obj, nullptr);
            EXPECT_EQ(obj->getName().getLexme(), "arr");
            auto* idx = dynamic_cast<LiteralExpr*>(set->getIndex().get());
            ASSERT_NE(idx, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(idx->getValue()), 1.0);
            auto* val = dynamic_cast<LiteralExpr*>(set->getValue().get());
            ASSERT_NE(val, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(val->getValue()), 5.0);
            });
        stmts[1]->accept(mock);
    }
}

TEST_F(ParserFixture, FuncWithArrayParam) {
    auto stmts = parse({
        // Func getFirst(arr) { return arr[0]; }
        Token(TokenType::FUNC,         "Func",     nullptr, 1),
        Token(TokenType::IDENTIFIER,   "getFirst", nullptr, 1),
        Token(TokenType::LEFT_PAREN,   "(",        nullptr, 1),
        Token(TokenType::IDENTIFIER,   "arr",      nullptr, 1),
        Token(TokenType::RIGHT_PAREN,  ")",        nullptr, 1),
        Token(TokenType::LEFT_BRACE,   "{",        nullptr, 2),
        Token(TokenType::RETURN,       "return",   nullptr, 3),
        Token(TokenType::IDENTIFIER,   "arr",      nullptr, 3),
        Token(TokenType::LEFT_BRACKET, "[",        nullptr, 3),
        Token(TokenType::NUMBER,       "0",        0.0,     3),
        Token(TokenType::RIGHT_BRACKET,"]",        nullptr, 3),
        Token(TokenType::SEMICOLON,    ";",        nullptr, 3),
        Token(TokenType::RIGHT_BRACE,  "}",        nullptr, 4),
        // var myArr = Array(3);
        Token(TokenType::VAR,          "var",      nullptr, 5),
        Token(TokenType::IDENTIFIER,   "myArr",    nullptr, 5),
        Token(TokenType::EQUAL,        "=",        nullptr, 5),
        Token(TokenType::ARRAY,        "Array",    nullptr, 5),
        Token(TokenType::LEFT_PAREN,   "(",        nullptr, 5),
        Token(TokenType::NUMBER,       "3",        3.0,     5),
        Token(TokenType::RIGHT_PAREN,  ")",        nullptr, 5),
        Token(TokenType::SEMICOLON,    ";",        nullptr, 5),
        // var result = getFirst(myArr);
        Token(TokenType::VAR,          "var",      nullptr, 6),
        Token(TokenType::IDENTIFIER,   "result",   nullptr, 6),
        Token(TokenType::EQUAL,        "=",        nullptr, 6),
        Token(TokenType::IDENTIFIER,   "getFirst", nullptr, 6),
        Token(TokenType::LEFT_PAREN,   "(",        nullptr, 6),
        Token(TokenType::IDENTIFIER,   "myArr",    nullptr, 6),
        Token(TokenType::RIGHT_PAREN,  ")",        nullptr, 6),
        Token(TokenType::SEMICOLON,    ";",        nullptr, 6),
        // print result;
        Token(TokenType::PRINT,        "print",    nullptr, 7),
        Token(TokenType::IDENTIFIER,   "result",   nullptr, 7),
        Token(TokenType::SEMICOLON,    ";",        nullptr, 7),
        Token(TokenType::EOF_TOKEN,    "",         nullptr, 7),
    });

    ASSERT_EQ(stmts.size(), 4u);

    // Func getFirst(arr) { return arr[0]; }
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitFuncStmt(testing::_))
            .WillOnce([](FuncStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "getFirst");
            ASSERT_EQ(stmt.getParams().size(), 1u);
            EXPECT_EQ(stmt.getParams()[0].getLexme(), "arr");
            ASSERT_EQ(stmt.getBody().size(), 1u);
            auto* ret = dynamic_cast<ReturnStmt*>(stmt.getBody()[0].get());
            ASSERT_NE(ret, nullptr);
            auto* get = dynamic_cast<ArrIndexGetExpr*>(ret->getValue().get());
            ASSERT_NE(get, nullptr);
            auto* obj = dynamic_cast<VariableExpr*>(get->getObject().get());
            ASSERT_NE(obj, nullptr);
            EXPECT_EQ(obj->getName().getLexme(), "arr");
            auto* idx = dynamic_cast<LiteralExpr*>(get->getIndex().get());
            ASSERT_NE(idx, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(idx->getValue()), 0.0);
            });
        stmts[0]->accept(mock);
    }
    // var myArr = Array(3);
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "myArr");
            auto* call = dynamic_cast<CallExpr*>(stmt.getInitializer().get());
            ASSERT_NE(call, nullptr);
            EXPECT_EQ(dynamic_cast<VariableExpr*>(call->getCallee().get())->getName().getLexme(), "Array");
            ASSERT_EQ(call->getArguments().size(), 1u);
            auto* size = dynamic_cast<LiteralExpr*>(call->getArguments()[0].get());
            ASSERT_NE(size, nullptr);
            EXPECT_DOUBLE_EQ(std::get<double>(size->getValue()), 3.0);
            });
        stmts[1]->accept(mock);
    }
    // var result = getFirst(myArr);
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitVarStmt(testing::_))
            .WillOnce([](VarStmt& stmt) {
            EXPECT_EQ(stmt.getName().getLexme(), "result");
            auto* call = dynamic_cast<CallExpr*>(stmt.getInitializer().get());
            ASSERT_NE(call, nullptr);
            EXPECT_EQ(dynamic_cast<VariableExpr*>(call->getCallee().get())->getName().getLexme(), "getFirst");
            ASSERT_EQ(call->getArguments().size(), 1u);
            auto* arg = dynamic_cast<VariableExpr*>(call->getArguments()[0].get());
            ASSERT_NE(arg, nullptr);
            EXPECT_EQ(arg->getName().getLexme(), "myArr");
            });
        stmts[2]->accept(mock);
    }
    // print result;
    {
        MockStmtVisitor mock;
        EXPECT_CALL(mock, visitPrintStmt(testing::_))
            .WillOnce([](PrintStmt& stmt) {
            auto* var = dynamic_cast<VariableExpr*>(stmt.getExpression().get());
            ASSERT_NE(var, nullptr);
            EXPECT_EQ(var->getName().getLexme(), "result");
            });
        stmts[3]->accept(mock);
    }
}

TEST_F(ParserFixture, ForAllClausesOmitted) {
    auto stmts = parse({
        // for (;;) { print 1; }
        Token(TokenType::FOR,         "for",   nullptr, 1),
        Token(TokenType::LEFT_PAREN,  "(",     nullptr, 1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 1),
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 1),
        Token(TokenType::PRINT,       "print", nullptr, 2),
        Token(TokenType::NUMBER,      "1",     1.0,     2),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 2),
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 3),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 3),
    });

    ASSERT_EQ(stmts.size(), 1u);

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitForStmt(testing::_))
        .WillOnce([](ForStmt& stmt) {
        EXPECT_EQ(stmt.getInitializer().get(), nullptr);
        EXPECT_EQ(stmt.getCondition().get(),   nullptr);
        EXPECT_EQ(stmt.getIncrement().get(),   nullptr);
        auto* body = dynamic_cast<BlockStmt*>(stmt.getBody().get());
        ASSERT_NE(body, nullptr);
        ASSERT_EQ(body->getStatements().size(), 1u);
        auto* print = dynamic_cast<PrintStmt*>(body->getStatements()[0].get());
        ASSERT_NE(print, nullptr);
        auto* lit = dynamic_cast<LiteralExpr*>(print->getExpression().get());
        ASSERT_NE(lit, nullptr);
        EXPECT_DOUBLE_EQ(std::get<double>(lit->getValue()), 1.0);
        });
    stmts[0]->accept(mock);
}

TEST_F(ParserFixture, ForInitOmitted) {
    auto stmts = parse({
        // for (; a < 5; a = a + 1) { print a; }
        Token(TokenType::FOR,         "for",   nullptr, 1),
        Token(TokenType::LEFT_PAREN,  "(",     nullptr, 1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::LESS,        "<",     nullptr, 1),
        Token(TokenType::NUMBER,      "5",     5.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::EQUAL,       "=",     nullptr, 1),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 1),
        Token(TokenType::PLUS,        "+",     nullptr, 1),
        Token(TokenType::NUMBER,      "1",     1.0,     1),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 1),
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 1),
        Token(TokenType::PRINT,       "print", nullptr, 2),
        Token(TokenType::IDENTIFIER,  "a",     nullptr, 2),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 2),
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 3),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 3),
    });

    ASSERT_EQ(stmts.size(), 1u);

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitForStmt(testing::_))
        .WillOnce([](ForStmt& stmt) {
        EXPECT_EQ(stmt.getInitializer().get(), nullptr);
        auto* cond = dynamic_cast<BinaryExpr*>(stmt.getCondition().get());
        ASSERT_NE(cond, nullptr);
        EXPECT_EQ(cond->getOp().getTokenType(), TokenType::LESS);
        auto* condLeft = dynamic_cast<VariableExpr*>(cond->getLeft().get());
        ASSERT_NE(condLeft, nullptr);
        EXPECT_EQ(condLeft->getName().getLexme(), "a");
        auto* condRight = dynamic_cast<LiteralExpr*>(cond->getRight().get());
        ASSERT_NE(condRight, nullptr);
        EXPECT_DOUBLE_EQ(std::get<double>(condRight->getValue()), 5.0);
        auto* incr = dynamic_cast<AssignExpr*>(stmt.getIncrement().get());
        ASSERT_NE(incr, nullptr);
        EXPECT_EQ(incr->getName().getLexme(), "a");
        });
    stmts[0]->accept(mock);
}

TEST_F(ParserFixture, ForIncrementOmitted) {
    auto stmts = parse({
        // for (var i = 0; i < 5;) { print i; }
        Token(TokenType::FOR,         "for",   nullptr, 1),
        Token(TokenType::LEFT_PAREN,  "(",     nullptr, 1),
        Token(TokenType::VAR,         "var",   nullptr, 1),
        Token(TokenType::IDENTIFIER,  "i",     nullptr, 1),
        Token(TokenType::EQUAL,       "=",     nullptr, 1),
        Token(TokenType::NUMBER,      "0",     0.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::IDENTIFIER,  "i",     nullptr, 1),
        Token(TokenType::LESS,        "<",     nullptr, 1),
        Token(TokenType::NUMBER,      "5",     5.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 1),
        Token(TokenType::LEFT_BRACE,  "{",     nullptr, 1),
        Token(TokenType::PRINT,       "print", nullptr, 2),
        Token(TokenType::IDENTIFIER,  "i",     nullptr, 2),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 2),
        Token(TokenType::RIGHT_BRACE, "}",     nullptr, 3),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 3),
    });

    ASSERT_EQ(stmts.size(), 1u);

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitForStmt(testing::_))
        .WillOnce([](ForStmt& stmt) {
        auto* init = dynamic_cast<VarStmt*>(stmt.getInitializer().get());
        ASSERT_NE(init, nullptr);
        EXPECT_EQ(init->getName().getLexme(), "i");
        auto* initVal = dynamic_cast<LiteralExpr*>(init->getInitializer().get());
        ASSERT_NE(initVal, nullptr);
        EXPECT_DOUBLE_EQ(std::get<double>(initVal->getValue()), 0.0);
        auto* cond = dynamic_cast<BinaryExpr*>(stmt.getCondition().get());
        ASSERT_NE(cond, nullptr);
        EXPECT_EQ(cond->getOp().getTokenType(), TokenType::LESS);
        auto* condLeft = dynamic_cast<VariableExpr*>(cond->getLeft().get());
        ASSERT_NE(condLeft, nullptr);
        EXPECT_EQ(condLeft->getName().getLexme(), "i");
        EXPECT_EQ(stmt.getIncrement().get(), nullptr);
        });
    stmts[0]->accept(mock);
}

TEST_F(ParserFixture, PrintUnaryMinusPlusLiteral) {
    auto stmts = parse({
        // print -3 + 2;
        Token(TokenType::PRINT,     "print", nullptr, 1),
        Token(TokenType::MINUS,     "-",     nullptr, 1),
        Token(TokenType::NUMBER,    "3",     3.0,     1),
        Token(TokenType::PLUS,      "+",     nullptr, 1),
        Token(TokenType::NUMBER,    "2",     2.0,     1),
        Token(TokenType::SEMICOLON, ";",     nullptr, 1),
        Token(TokenType::EOF_TOKEN, "",      nullptr, 1),
    });

    ASSERT_EQ(stmts.size(), 1u);

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitPrintStmt(testing::_))
        .WillOnce([](PrintStmt& stmt) {
        auto* add = dynamic_cast<BinaryExpr*>(stmt.getExpression().get());
        ASSERT_NE(add, nullptr);
        EXPECT_EQ(add->getOp().getTokenType(), TokenType::PLUS);
        auto* unary = dynamic_cast<UnaryExpr*>(add->getLeft().get());
        ASSERT_NE(unary, nullptr);
        EXPECT_EQ(unary->getOp().getTokenType(), TokenType::MINUS);
        auto* three = dynamic_cast<LiteralExpr*>(unary->getRight().get());
        ASSERT_NE(three, nullptr);
        EXPECT_DOUBLE_EQ(std::get<double>(three->getValue()), 3.0);
        auto* two = dynamic_cast<LiteralExpr*>(add->getRight().get());
        ASSERT_NE(two, nullptr);
        EXPECT_DOUBLE_EQ(std::get<double>(two->getValue()), 2.0);
        });
    stmts[0]->accept(mock);
}

TEST_F(ParserFixture, PrintStringConcatenation) {
    auto stmts = parse({
        // print "abc" + "def";
        Token(TokenType::PRINT,     "print", nullptr,            1),
        Token(TokenType::STRING,    "abc",   std::string("abc"), 1),
        Token(TokenType::PLUS,      "+",     nullptr,            1),
        Token(TokenType::STRING,    "def",   std::string("def"), 1),
        Token(TokenType::SEMICOLON, ";",     nullptr,            1),
        Token(TokenType::EOF_TOKEN, "",      nullptr,            1),
    });

    ASSERT_EQ(stmts.size(), 1u);

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitPrintStmt(testing::_))
        .WillOnce([](PrintStmt& stmt) {
        auto* bin = dynamic_cast<BinaryExpr*>(stmt.getExpression().get());
        ASSERT_NE(bin, nullptr);
        EXPECT_EQ(bin->getOp().getTokenType(), TokenType::PLUS);
        auto* left = dynamic_cast<LiteralExpr*>(bin->getLeft().get());
        ASSERT_NE(left, nullptr);
        ASSERT_TRUE(std::holds_alternative<std::string>(left->getValue()));
        EXPECT_EQ(std::get<std::string>(left->getValue()), "abc");
        auto* right = dynamic_cast<LiteralExpr*>(bin->getRight().get());
        ASSERT_NE(right, nullptr);
        ASSERT_TRUE(std::holds_alternative<std::string>(right->getValue()));
        EXPECT_EQ(std::get<std::string>(right->getValue()), "def");
        });
    stmts[0]->accept(mock);
}

TEST_F(ParserFixture, GroupingExpr) {
    auto stmts = parse({
        // print (1 + 2) * 3;
        Token(TokenType::PRINT,       "print", nullptr, 1),
        Token(TokenType::LEFT_PAREN,  "(",     nullptr, 1),
        Token(TokenType::NUMBER,      "1",     1.0,     1),
        Token(TokenType::PLUS,        "+",     nullptr, 1),
        Token(TokenType::NUMBER,      "2",     2.0,     1),
        Token(TokenType::RIGHT_PAREN, ")",     nullptr, 1),
        Token(TokenType::STAR,        "*",     nullptr, 1),
        Token(TokenType::NUMBER,      "3",     3.0,     1),
        Token(TokenType::SEMICOLON,   ";",     nullptr, 1),
        Token(TokenType::EOF_TOKEN,   "",      nullptr, 1),
    });

    ASSERT_EQ(stmts.size(), 1u);

    MockStmtVisitor mock;
    EXPECT_CALL(mock, visitPrintStmt(testing::_))
        .WillOnce([](PrintStmt& stmt) {
        auto* mul = dynamic_cast<BinaryExpr*>(stmt.getExpression().get());
        ASSERT_NE(mul, nullptr);
        EXPECT_EQ(mul->getOp().getTokenType(), TokenType::STAR);
        auto* group = dynamic_cast<GroupingExpr*>(mul->getLeft().get());
        ASSERT_NE(group, nullptr);
        auto* add = dynamic_cast<BinaryExpr*>(group->getExpression().get());
        ASSERT_NE(add, nullptr);
        EXPECT_EQ(add->getOp().getTokenType(), TokenType::PLUS);
        auto* addLeft = dynamic_cast<LiteralExpr*>(add->getLeft().get());
        ASSERT_NE(addLeft, nullptr);
        EXPECT_DOUBLE_EQ(std::get<double>(addLeft->getValue()), 1.0);
        auto* addRight = dynamic_cast<LiteralExpr*>(add->getRight().get());
        ASSERT_NE(addRight, nullptr);
        EXPECT_DOUBLE_EQ(std::get<double>(addRight->getValue()), 2.0);
        auto* three = dynamic_cast<LiteralExpr*>(mul->getRight().get());
        ASSERT_NE(three, nullptr);
        EXPECT_DOUBLE_EQ(std::get<double>(three->getValue()), 3.0);
        });
    stmts[0]->accept(mock);
}
