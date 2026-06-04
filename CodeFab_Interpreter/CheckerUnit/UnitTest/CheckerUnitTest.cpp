#include "gmock/gmock.h"
#include "../CheckerUnit.h"

using std::vector;

using namespace testing;

TEST(CheckerUnit, EmptyProgram) {
	CheckerUnit checker_unit;
	vector<StmtPtr> input = {};
	EXPECT_NO_THROW(checker_unit.DoChecker(input));
}

TEST(CheckerUnit, DeclareValue) {
    std::vector<StmtPtr> stmts;
    // var a = 1
    auto token = Token(TokenType::IDENTIFIER, "a", nullptr, 1);
    stmts.push_back(std::make_unique<VarStmt>(token, std::move(nullptr)));

    CheckerUnit checker_unit;
    EXPECT_NO_THROW(checker_unit.DoChecker(stmts));
}

TEST(CheckerUnit, DeclareDuplicateValueException) {
    // var a = 10;
    // var a = 20;
    std::vector<StmtPtr> stmts;

    auto token1 = Token(TokenType::IDENTIFIER, "a", nullptr, 1);
    auto token2 = Token(TokenType::IDENTIFIER, "a", nullptr, 2);
    auto literal_expr_10 = std::make_unique<LiteralExpr>(10.0);
    auto literal_expr_20 = std::make_unique<LiteralExpr>(20.0);
    stmts.push_back(std::make_unique<VarStmt>(token1, std::move(literal_expr_10)));
    stmts.push_back(std::make_unique<VarStmt>(token2, std::move(literal_expr_20)));

    CheckerUnit checker_unit;
    try {
        checker_unit.DoChecker(stmts);
        FAIL() << "The CheckerError should occur.";
    }
    catch (const CheckerError& e) {
        EXPECT_THAT(std::string(e.what()), HasSubstr("Already a variable with this name in this scope"));
    }
}