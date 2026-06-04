#include "gmock/gmock.h"
#include "../CheckerUnit.h"

using std::vector;
using namespace testing;

class CheckerUnitFixture : public Test
{
public:
	void SetUp() override {
		p_checker_unit = new CheckerUnit();
	}

	void TearDown() override {
		if (p_checker_unit) delete p_checker_unit;
		p_checker_unit = nullptr;
	}
protected:
	Token makeIndentifier(const std::string& name, int line = 1) {
		return Token(TokenType::IDENTIFIER, name, nullptr, line);
	}
	ExprPtr numLiteral(double value) {
		return std::make_unique<LiteralExpr>(value);
	}
	ExprPtr stringLiteral(string str) {
		return std::make_unique<LiteralExpr>(str);
	}
	StmtPtr valueDeclaration(const std::string& name, ExprPtr init = nullptr, int line = 1) {
		return std::make_unique<VarStmt>(makeIndentifier(name, line), std::move(init));
	}
	StmtPtr makeBlockStatement(std::vector<StmtPtr> statement) {
		return std::make_unique<BlockStmt>(std::move(statement));
	}

protected:
	CheckerUnit* p_checker_unit = nullptr;
	vector<StmtPtr> statement_vector = {};
};

TEST_F(CheckerUnitFixture, EmptyProgram) {
	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, DeclareValue) {
	// var a = 1
	statement_vector.push_back(valueDeclaration("a", numLiteral(1.0)));
	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, DeclareDuplicateValueException) {
	// var a = 10;
	// var a = 20;
	statement_vector.push_back(valueDeclaration("a", numLiteral(10.0), 1));
	statement_vector.push_back(valueDeclaration("a", numLiteral(20.0), 2));

	try {
		p_checker_unit->doChecker(statement_vector);
		FAIL() << "The CheckerError should occur.";
	}
	catch (const CheckerError& e) {
		EXPECT_THAT(std::string(e.what()), HasSubstr("Already a variable with this name in this scope"));
	}
}

TEST_F(CheckerUnitFixture, DeclareValueInBlock) {
	std::vector<StmtPtr> inner;
	inner.push_back(valueDeclaration("x", numLiteral(42.0)));
	auto block_statement = makeBlockStatement(std::move(inner));
	statement_vector.push_back(std::move(block_statement));
	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, DeclareSameNameInNestedScopes) {
	// { var a = 1; { var a = 2; } }
	std::vector<StmtPtr> inner, outer;
	inner.push_back(valueDeclaration("a", numLiteral(2.0)));
	auto inner_block_statement = makeBlockStatement(std::move(inner));

	outer.push_back(valueDeclaration("a", numLiteral(1.0)));
	outer.push_back(std::move(inner_block_statement));

	auto outer_block_statement = makeBlockStatement(std::move(outer));
	statement_vector.push_back(std::move(outer_block_statement));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, PrintNumberLiteral) {
	// print 3.14;
	statement_vector.push_back(std::make_unique<PrintStmt>(numLiteral(3.14)));
	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, PrintStringLiteral) {
	// print "hello";
	statement_vector.push_back(std::make_unique<PrintStmt>(stringLiteral("hello")));
	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}
