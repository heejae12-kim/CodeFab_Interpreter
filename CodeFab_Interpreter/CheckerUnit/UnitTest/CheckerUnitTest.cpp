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
	ExprPtr boolLiteral(bool value) {
		return std::make_unique<LiteralExpr>(value);
	}
	ExprPtr nullLiteral() {
		return std::make_unique<LiteralExpr>(ValuableValue{ nullptr });
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
	std::vector<StmtPtr> statement_body_vector = {};
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

TEST_F(CheckerUnitFixture, UnaryExpressionNegativeNumber) {
	// print -1;
	statement_vector.push_back(std::make_unique<PrintStmt>(
		std::make_unique<UnaryExpr>(
			Token(TokenType::MINUS, "-", nullptr, 1),
			numLiteral(1.0)
		)
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, BinaryExpressiongAddition) {
	// print 1 + 2;
	statement_vector.push_back(std::make_unique<PrintStmt>(
		std::make_unique<BinaryExpr>(
			numLiteral(1.0),
			Token(TokenType::PLUS, "+", nullptr, 1),
			numLiteral(2.0)
		)
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, SelfInitializerThrows) {
	// { var a = a; }
	std::vector<StmtPtr> inner;
	inner.push_back(std::make_unique<VarStmt>(
		makeIndentifier("a", 1),
		std::make_unique<VariableExpr>(makeIndentifier("a", 1))
	));
	statement_vector.push_back(makeBlockStatement(std::move(inner)));

	try {
		p_checker_unit->doChecker(statement_vector);
		FAIL() << "The CheckerError should occur.";
	}
	catch (const CheckerError& e) {
		EXPECT_THAT(std::string(e.what()), HasSubstr("Can't read local variable in its own initializer"));
	}
}

TEST_F(CheckerUnitFixture, CheckIfAndElseStatement) {
	// if (1) { var x = 1; } else { var x = 2; }
	std::vector<StmtPtr> thenInner;
	thenInner.push_back(valueDeclaration("x", numLiteral(1.0)));

	std::vector<StmtPtr> elseInner;
	elseInner.push_back(valueDeclaration("x", numLiteral(2.0)));

	statement_vector.push_back(std::make_unique<IfStmt>(
		numLiteral(1.0),
		makeBlockStatement(std::move(thenInner)),
		makeBlockStatement(std::move(elseInner))
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, IfStmtBinaryExpressiongInCondition) {
	// var x = 2; if (x > 1) { print x; }
	std::vector<StmtPtr> thenInner;
	thenInner.push_back(std::make_unique<PrintStmt>(
		std::make_unique<VariableExpr>(makeIndentifier("x"))
	));

	statement_vector.push_back(valueDeclaration("x", numLiteral(2.0)));
	statement_vector.push_back(std::make_unique<IfStmt>(
		std::make_unique<BinaryExpr>(
			std::make_unique<VariableExpr>(makeIndentifier("x")),
			Token(TokenType::GREATER, ">", nullptr, 1),
			numLiteral(1.0)
		),
		makeBlockStatement(std::move(thenInner)),
		nullptr
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}


TEST_F(CheckerUnitFixture, ForStmtFullLoop) {
	// for (var i = 0; i < 3; i = i + 1) { print i; }
	statement_body_vector.push_back(std::make_unique<PrintStmt>(
		std::make_unique<VariableExpr>(makeIndentifier("i"))
	));

	statement_vector.push_back(std::make_unique<ForStmt>(
		valueDeclaration("i", numLiteral(0.0)),
		std::make_unique<BinaryExpr>(
			std::make_unique<VariableExpr>(makeIndentifier("i")),
			Token(TokenType::LESS, "<", nullptr, 1),
			numLiteral(3.0)
		),
		std::make_unique<AssignExpr>(
			makeIndentifier("i"),
			std::make_unique<BinaryExpr>(
				std::make_unique<VariableExpr>(makeIndentifier("i")),
				Token(TokenType::PLUS, "+", nullptr, 1),
				numLiteral(1.0)
			)
		),
		makeBlockStatement(std::move(statement_body_vector))
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, UnaryExpressionBang) {
	// print !true;
	statement_vector.push_back(std::make_unique<PrintStmt>(
		std::make_unique<UnaryExpr>(
			Token(TokenType::BANG, "!", nullptr, 1),
			boolLiteral(true)
		)
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, BinaryExpressionAndOp) {
	// print true and false;
	statement_vector.push_back(std::make_unique<PrintStmt>(
		std::make_unique<BinaryExpr>(
			boolLiteral(true),
			Token(TokenType::AND_OP, "and", nullptr, 1),
			boolLiteral(false)
		)
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, BinaryExpressionOrOp) {
	// print true or false;
	statement_vector.push_back(std::make_unique<PrintStmt>(
		std::make_unique<BinaryExpr>(
			boolLiteral(true),
			Token(TokenType::OR_OP, "or", nullptr, 1),
			boolLiteral(false)
		)
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, CallExprWithArgs) {
	// foo(1, "hello")
	std::vector<ExprPtr> args;
	args.push_back(numLiteral(1.0));
	args.push_back(stringLiteral("hello"));

	statement_vector.push_back(std::make_unique<ExprStmt>(
		std::make_unique<CallExpr>(
			std::make_unique<VariableExpr>(makeIndentifier("foo")),
			Token(TokenType::LEFT_PAREN, "(", nullptr, 1),
			std::move(args)
		)
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, FuncDeclareNoParams) {
	// func foo() { }
	statement_vector.push_back(std::make_unique<FuncStmt>(
		makeIndentifier("foo"),
		std::vector<Token>{},
		std::vector<StmtPtr>{}
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, FuncDeclareWithParams) {
	// func foo(a, b) { }
	statement_vector.push_back(std::make_unique<FuncStmt>(
		makeIndentifier("foo"),
		std::vector<Token>{ makeIndentifier("a"), makeIndentifier("b") },
		std::vector<StmtPtr>{}
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, FuncDuplicateParamThrows) {
	// func foo(a, a) { }
	statement_vector.push_back(std::make_unique<FuncStmt>(
		makeIndentifier("foo"),
		std::vector<Token>{ makeIndentifier("a"), makeIndentifier("a") },
		std::vector<StmtPtr>{}
	));

	try {
		p_checker_unit->doChecker(statement_vector);
		FAIL() << "The CheckerError should occur.";
	}
	catch (const CheckerError& e) {
		EXPECT_THAT(std::string(e.what()), HasSubstr("Duplicate parameter name"));
	}
}

TEST_F(CheckerUnitFixture, FuncBodyCanAccessParams) {
	// func foo(x) { print x; }
	statement_body_vector.push_back(std::make_unique<PrintStmt>(
		std::make_unique<VariableExpr>(makeIndentifier("x"))
	));

	statement_vector.push_back(std::make_unique<FuncStmt>(
		makeIndentifier("foo"),
		std::vector<Token>{ makeIndentifier("x") },
		std::move(statement_body_vector)
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, ReturnInsideFunction) {
	// func foo() { return 1; }
	statement_body_vector.push_back(std::make_unique<ReturnStmt>(
		Token(TokenType::IDENTIFIER, "return", nullptr, 1),
		numLiteral(1.0)
	));

	statement_vector.push_back(std::make_unique<FuncStmt>(
		makeIndentifier("foo"),
		std::vector<Token>{},
		std::move(statement_body_vector)
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, ReturnAtTopLevelThrows) {
	// return 1;
	statement_vector.push_back(std::make_unique<ReturnStmt>(
		Token(TokenType::IDENTIFIER, "return", nullptr, 1),
		numLiteral(1.0)
	));

	try {
		p_checker_unit->doChecker(statement_vector);
		FAIL() << "The CheckerError should occur.";
	}
	catch (const CheckerError& e) {
		EXPECT_THAT(std::string(e.what()), HasSubstr("Can't return from top-level code"));
	}
}

TEST_F(CheckerUnitFixture, ReturnWithoutValue) {
	// func foo() { return; }
	statement_body_vector.push_back(std::make_unique<ReturnStmt>(
		Token(TokenType::IDENTIFIER, "return", nullptr, 1),
		nullptr
	));

	statement_vector.push_back(std::make_unique<FuncStmt>(
		makeIndentifier("foo"),
		std::vector<Token>{},
		std::move(statement_body_vector)
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, IndexGetExprLiteralIndex) {
	// arr[0];
	statement_vector.push_back(std::make_unique<ExprStmt>(
		std::make_unique<ArrIndexGetExpr>(
			std::make_unique<VariableExpr>(makeIndentifier("arr")),
			numLiteral(0.0),
			Token(TokenType::LEFT_PAREN, "[", nullptr, 1)
		)
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, IndexGetExprVariableIndex) {
	// var i = 1; arr[i];
	statement_vector.push_back(valueDeclaration("i", numLiteral(1.0)));
	statement_vector.push_back(std::make_unique<ExprStmt>(
		std::make_unique<ArrIndexGetExpr>(
			std::make_unique<VariableExpr>(makeIndentifier("arr")),
			std::make_unique<VariableExpr>(makeIndentifier("i")),
			Token(TokenType::LEFT_PAREN, "[", nullptr, 1)
		)
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, IndexSetExprLiteralValue) {
	// arr[0] = 1;
	statement_vector.push_back(std::make_unique<ExprStmt>(
		std::make_unique<ArrIndexSetExpr>(
			std::make_unique<VariableExpr>(makeIndentifier("arr")),
			numLiteral(0.0),
			Token(TokenType::LEFT_PAREN, "[", nullptr, 1),
			numLiteral(1.0)
		)
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

TEST_F(CheckerUnitFixture, IndexSetExprVariableValue) {
	// var x = 1; arr[0] = x;
	statement_vector.push_back(valueDeclaration("x", numLiteral(1.0)));
	statement_vector.push_back(std::make_unique<ExprStmt>(
		std::make_unique<ArrIndexSetExpr>(
			std::make_unique<VariableExpr>(makeIndentifier("arr")),
			numLiteral(0.0),
			Token(TokenType::LEFT_PAREN, "[", nullptr, 1),
			std::make_unique<VariableExpr>(makeIndentifier("x"))
		)
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}

// ========== evaluateUnaryCalc ==========

TEST_F(CheckerUnitFixture, UnaryCalcNegatesNumber) {
	// var a = -5.0;  ->  folded to -5.0
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<UnaryExpr>(Token(TokenType::MINUS, "-", nullptr, 1), numLiteral(5.0))
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<double>(folded->getValue()), -5.0);
}

TEST_F(CheckerUnitFixture, UnaryCalcBangOnTrue) {
	// var a = !true;  ->  folded to false
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<UnaryExpr>(Token(TokenType::BANG, "!", nullptr, 1), boolLiteral(true))
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<bool>(folded->getValue()), false);
}

TEST_F(CheckerUnitFixture, UnaryCalcBangOnFalse) {
	// var a = !false;  ->  folded to true
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<UnaryExpr>(Token(TokenType::BANG, "!", nullptr, 1), boolLiteral(false))
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<bool>(folded->getValue()), true);
}

TEST_F(CheckerUnitFixture, UnaryCalcBangOnNil) {
	// var a = !nil;  ->  folded to true
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<UnaryExpr>(Token(TokenType::BANG, "!", nullptr, 1), nullLiteral())
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<bool>(folded->getValue()), true);
}

TEST_F(CheckerUnitFixture, UnaryCalcBangOnNonBoolThrows) {
	// var a = !1;  ->  CheckerError
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<UnaryExpr>(Token(TokenType::BANG, "!", nullptr, 1), numLiteral(1.0))
	);
	statement_vector.push_back(std::move(var_stmt));

	try {
		p_checker_unit->doChecker(statement_vector);
		FAIL() << "The CheckerError should occur.";
	}
	catch (const CheckerError& e) {
		EXPECT_THAT(std::string(e.what()), HasSubstr("Operand must be a boolean or nullptr"));
	}
}

TEST_F(CheckerUnitFixture, UnaryCalcMinusOnStringThrows) {
	// var a = -"hello";  ->  CheckerError
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<UnaryExpr>(Token(TokenType::MINUS, "-", nullptr, 1), stringLiteral("hello"))
	);
	statement_vector.push_back(std::move(var_stmt));

	try {
		p_checker_unit->doChecker(statement_vector);
		FAIL() << "The CheckerError should occur.";
	}
	catch (const CheckerError& e) {
		EXPECT_THAT(std::string(e.what()), HasSubstr("Operand must be a number"));
	}
}

// ========== evaluateBinaryCalc ==========

TEST_F(CheckerUnitFixture, BinaryCalcAddsNumbers) {
	// var a = 1 + 2;  ->  folded to 3.0
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(numLiteral(1.0), Token(TokenType::PLUS, "+", nullptr, 1), numLiteral(2.0))
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<double>(folded->getValue()), 3.0);
}

TEST_F(CheckerUnitFixture, BinaryCalcConcatenatesStrings) {
	// var a = "hello" + " world";  ->  folded to "hello world"
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(stringLiteral("hello"), Token(TokenType::PLUS, "+", nullptr, 1), stringLiteral(" world"))
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<std::string>(folded->getValue()), "hello world");
}

TEST_F(CheckerUnitFixture, BinaryCalcSubtractsNumbers) {
	// var a = 5 - 3;  ->  folded to 2.0
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(numLiteral(5.0), Token(TokenType::MINUS, "-", nullptr, 1), numLiteral(3.0))
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<double>(folded->getValue()), 2.0);
}

TEST_F(CheckerUnitFixture, BinaryCalcMultipliesNumbers) {
	// var a = 3 * 4;  ->  folded to 12.0
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(numLiteral(3.0), Token(TokenType::STAR, "*", nullptr, 1), numLiteral(4.0))
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<double>(folded->getValue()), 12.0);
}

TEST_F(CheckerUnitFixture, BinaryCalcDividesNumbers) {
	// var a = 10 / 2;  ->  folded to 5.0
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(numLiteral(10.0), Token(TokenType::SLASH, "/", nullptr, 1), numLiteral(2.0))
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<double>(folded->getValue()), 5.0);
}

TEST_F(CheckerUnitFixture, BinaryCalcLessComparison) {
	// var a = 1 < 2;  ->  folded to true
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(numLiteral(1.0), Token(TokenType::LESS, "<", nullptr, 1), numLiteral(2.0))
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<bool>(folded->getValue()), true);
}

TEST_F(CheckerUnitFixture, BinaryCalcEqualEqualNumbers) {
	// var a = 1 == 1;  ->  folded to true
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(numLiteral(1.0), Token(TokenType::EQUAL_EQUAL, "==", nullptr, 1), numLiteral(1.0))
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<bool>(folded->getValue()), true);
}

TEST_F(CheckerUnitFixture, BinaryCalcBangEqualNumbers) {
	// var a = 1 != 2;  ->  folded to true
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(numLiteral(1.0), Token(TokenType::BANG_EQUAL, "!=", nullptr, 1), numLiteral(2.0))
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<bool>(folded->getValue()), true);
}

TEST_F(CheckerUnitFixture, BinaryCalcAndOp) {
	// var a = true && false;  ->  folded to false
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(boolLiteral(true), Token(TokenType::AND_OP, "&&", nullptr, 1), boolLiteral(false))
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<bool>(folded->getValue()), false);
}

TEST_F(CheckerUnitFixture, BinaryCalcOrOp) {
	// var a = false || true;  ->  folded to true
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(boolLiteral(false), Token(TokenType::OR_OP, "||", nullptr, 1), boolLiteral(true))
	);
	auto* raw = var_stmt.get();
	statement_vector.push_back(std::move(var_stmt));

	p_checker_unit->doChecker(statement_vector);

	auto* folded = dynamic_cast<LiteralExpr*>(raw->getInitializer().get());
	ASSERT_NE(folded, nullptr);
	EXPECT_EQ(std::get<bool>(folded->getValue()), true);
}

TEST_F(CheckerUnitFixture, BinaryCalcDivisionByZeroThrows) {
	// var a = 1 / 0;  ->  CheckerError
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(numLiteral(1.0), Token(TokenType::SLASH, "/", nullptr, 1), numLiteral(0.0))
	);
	statement_vector.push_back(std::move(var_stmt));

	try {
		p_checker_unit->doChecker(statement_vector);
		FAIL() << "The CheckerError should occur.";
	}
	catch (const CheckerError& e) {
		EXPECT_THAT(std::string(e.what()), HasSubstr("Division by zero"));
	}
}

TEST_F(CheckerUnitFixture, BinaryCalcTypeMismatchThrows) {
	// var a = 1 + "a";  ->  CheckerError
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(numLiteral(1.0), Token(TokenType::PLUS, "+", nullptr, 1), stringLiteral("a"))
	);
	statement_vector.push_back(std::move(var_stmt));

	try {
		p_checker_unit->doChecker(statement_vector);
		FAIL() << "The CheckerError should occur.";
	}
	catch (const CheckerError& e) {
		EXPECT_THAT(std::string(e.what()), HasSubstr("Operands must be two numbers or two strings"));
	}
}

TEST_F(CheckerUnitFixture, BinaryCalcEqualEqualTypeMismatchThrows) {
	// var a = 1 == "a";  ->  CheckerError
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(numLiteral(1.0), Token(TokenType::EQUAL_EQUAL, "==", nullptr, 1), stringLiteral("a"))
	);
	statement_vector.push_back(std::move(var_stmt));

	try {
		p_checker_unit->doChecker(statement_vector);
		FAIL() << "The CheckerError should occur.";
	}
	catch (const CheckerError& e) {
		EXPECT_THAT(std::string(e.what()), HasSubstr("Operands must be the same type"));
	}
}

TEST_F(CheckerUnitFixture, BinaryCalcBangEqualTypeMismatchThrows) {
	// var a = 1 != "a";  ->  CheckerError
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(numLiteral(1.0), Token(TokenType::BANG_EQUAL, "!=", nullptr, 1), stringLiteral("a"))
	);
	statement_vector.push_back(std::move(var_stmt));

	try {
		p_checker_unit->doChecker(statement_vector);
		FAIL() << "The CheckerError should occur.";
	}
	catch (const CheckerError& e) {
		EXPECT_THAT(std::string(e.what()), HasSubstr("Operands must be the same type"));
	}
}

TEST_F(CheckerUnitFixture, BinaryCalcAndOpNonBoolThrows) {
	// var a = 1 && true;  ->  CheckerError
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(numLiteral(1.0), Token(TokenType::AND_OP, "&&", nullptr, 1), boolLiteral(true))
	);
	statement_vector.push_back(std::move(var_stmt));

	try {
		p_checker_unit->doChecker(statement_vector);
		FAIL() << "The CheckerError should occur.";
	}
	catch (const CheckerError& e) {
		EXPECT_THAT(std::string(e.what()), HasSubstr("Operands must be booleans"));
	}
}

TEST_F(CheckerUnitFixture, BinaryCalcOrOpNonBoolThrows) {
	// var a = "hello" || false;  ->  CheckerError
	auto var_stmt = std::make_unique<VarStmt>(
		makeIndentifier("a"),
		std::make_unique<BinaryExpr>(stringLiteral("hello"), Token(TokenType::OR_OP, "||", nullptr, 1), boolLiteral(false))
	);
	statement_vector.push_back(std::move(var_stmt));

	try {
		p_checker_unit->doChecker(statement_vector);
		FAIL() << "The CheckerError should occur.";
	}
	catch (const CheckerError& e) {
		EXPECT_THAT(std::string(e.what()), HasSubstr("Operands must be booleans"));
	}
}

TEST_F(CheckerUnitFixture, ComplexExpressionWithPrecedence) {
	// var a = 1;
	// a = a + (1 - 2 * 3 * 4 * 5 / 6 + 7 + 8 + 9) * 10;
	statement_vector.push_back(valueDeclaration("a", numLiteral(1.0)));

	// 2 * 3 * 4 * 5 / 6
	auto mul_2_3 = std::make_unique<BinaryExpr>(
		numLiteral(2.0), Token(TokenType::STAR,  "*", nullptr, 2), numLiteral(3.0));
	auto mul_x_4 = std::make_unique<BinaryExpr>(
		std::move(mul_2_3), Token(TokenType::STAR,  "*", nullptr, 2), numLiteral(4.0));
	auto mul_x_5 = std::make_unique<BinaryExpr>(
		std::move(mul_x_4), Token(TokenType::STAR,  "*", nullptr, 2), numLiteral(5.0));
	auto div_x_6 = std::make_unique<BinaryExpr>(
		std::move(mul_x_5), Token(TokenType::SLASH, "/", nullptr, 2), numLiteral(6.0));

	// 1 - (2*3*4*5/6) + 7 + 8 + 9
	auto sub_1 = std::make_unique<BinaryExpr>(
		numLiteral(1.0), Token(TokenType::MINUS, "-", nullptr, 2), std::move(div_x_6));
	auto add_7 = std::make_unique<BinaryExpr>(
		std::move(sub_1), Token(TokenType::PLUS, "+", nullptr, 2), numLiteral(7.0));
	auto add_8 = std::make_unique<BinaryExpr>(
		std::move(add_7), Token(TokenType::PLUS, "+", nullptr, 2), numLiteral(8.0));
	auto add_9 = std::make_unique<BinaryExpr>(
		std::move(add_8), Token(TokenType::PLUS, "+", nullptr, 2), numLiteral(9.0));

	// (inner) * 10
	auto group   = std::make_unique<GroupingExpr>(std::move(add_9));
	auto mul_10  = std::make_unique<BinaryExpr>(
		std::move(group), Token(TokenType::STAR, "*", nullptr, 2), numLiteral(10.0));

	// a + (inner) * 10
	auto add_a = std::make_unique<BinaryExpr>(
		std::make_unique<VariableExpr>(makeIndentifier("a", 2)),
		Token(TokenType::PLUS, "+", nullptr, 2),
		std::move(mul_10));

	statement_vector.push_back(std::make_unique<ExprStmt>(
		std::make_unique<AssignExpr>(makeIndentifier("a", 2), std::move(add_a))
	));

	EXPECT_NO_THROW(p_checker_unit->doChecker(statement_vector));
}
