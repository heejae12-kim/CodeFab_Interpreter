#include "gmock/gmock.h"
#include "Interpreter.h"
#include "TestHelpers.h"

using namespace tst;

namespace {
    // AST(문장 목록)를 실행하고 print 로 출력된 내용을 문자열로 반환한다.
    // Lexer/Parser 가 아직 없으므로 소스 문자열 대신 AST 를 직접 넣는다.
    std::string run(std::vector<StmtPtr> program) {
        Interpreter interpreter;
        CoutCapture capture;
        interpreter.interpret(program);
        return capture.str();
    }

    // 단일 표현식을 print 로 감싸 실행한다.
    std::string runExpr(ExprPtr e) {
        return run(stmts(printStmt(std::move(e))));
    }
}

// ── 산술 / 우선순위 ─────────────────────────────────────────────────────────
TEST(InterpreterExprTest, ArithmeticBasic) {
    EXPECT_EQ(runExpr(binary(litNum(3.0), TokenType::PLUS,  "+", litNum(2.0))), "5\n");
    EXPECT_EQ(runExpr(binary(litNum(3.0), TokenType::MINUS, "-", litNum(2.0))), "1\n");
    EXPECT_EQ(runExpr(binary(litNum(3.0), TokenType::STAR,  "*", litNum(2.0))), "6\n");
    EXPECT_EQ(runExpr(binary(litNum(6.0), TokenType::SLASH, "/", litNum(2.0))), "3\n");
}

TEST(InterpreterExprTest, PrecedenceViaTreeShape) {
    // 1 + (2 * 3) = 7  (파서가 우선순위에 맞춰 만든 트리 형태)
    ExprPtr e = binary(litNum(1.0), TokenType::PLUS, "+",
        binary(litNum(2.0), TokenType::STAR, "*", litNum(3.0)));
    EXPECT_EQ(runExpr(std::move(e)), "7\n");
}

TEST(InterpreterExprTest, GroupingChangesPrecedence) {
    // (1 + 2) * 3 = 9
    ExprPtr e = binary(
        grouping(binary(litNum(1.0), TokenType::PLUS, "+", litNum(2.0))),
        TokenType::STAR, "*", litNum(3.0));
    EXPECT_EQ(runExpr(std::move(e)), "9\n");
}

TEST(InterpreterExprTest, IntegerValuedDoublePrintsWithoutDecimal) {
    EXPECT_EQ(runExpr(litNum(5.0)), "5\n");
}

// ── 문자열 ──────────────────────────────────────────────────────────────────
TEST(InterpreterExprTest, StringConcatenation) {
    EXPECT_EQ(runExpr(binary(litStr("foo"), TokenType::PLUS, "+", litStr("bar"))), "foobar\n");
}

// ── 단항 ────────────────────────────────────────────────────────────────────
TEST(InterpreterExprTest, UnaryMinusNegatesNumber) {
    EXPECT_EQ(runExpr(unary(TokenType::MINUS, "-", litNum(5.0))), "-5\n");
}

// ── 비교 / 동치 → 불리언 ────────────────────────────────────────────────────
TEST(InterpreterExprTest, ComparisonProducesBoolean) {
    EXPECT_EQ(runExpr(binary(litNum(5.0), TokenType::GREATER,       ">",  litNum(3.0))), "true\n");
    EXPECT_EQ(runExpr(binary(litNum(5.0), TokenType::LESS,          "<",  litNum(3.0))), "false\n");
    EXPECT_EQ(runExpr(binary(litNum(3.0), TokenType::GREATER_EQUAL, ">=", litNum(3.0))), "true\n");
    EXPECT_EQ(runExpr(binary(litNum(2.0), TokenType::LESS_EQUAL,    "<=", litNum(1.0))), "false\n");
}

TEST(InterpreterExprTest, EqualityAcrossTypes) {
    EXPECT_EQ(runExpr(binary(litNum(3.0), TokenType::EQUAL_EQUAL, "==", litNum(3.0))), "true\n");
    EXPECT_EQ(runExpr(binary(litNum(3.0), TokenType::BANG_EQUAL,  "!=", litNum(2.0))), "true\n");
    EXPECT_EQ(runExpr(binary(litNum(3.0), TokenType::EQUAL_EQUAL, "==", litStr("3"))), "false\n");
}

// ── 런타임 오류 ─────────────────────────────────────────────────────────────
TEST(InterpreterExprTest, DivisionByZeroThrows) {
    EXPECT_THROW(runExpr(binary(litNum(3.0), TokenType::SLASH, "/", litNum(0.0))), RuntimeError);
}

TEST(InterpreterExprTest, NumberMinusStringThrows) {
    // 3 - "hello"  (PDF p.86)
    EXPECT_THROW(runExpr(binary(litNum(3.0), TokenType::MINUS, "-", litStr("hello"))), RuntimeError);
}

TEST(InterpreterExprTest, PlusMixedTypesThrows) {
    EXPECT_THROW(runExpr(binary(litNum(3.0), TokenType::PLUS, "+", litStr("x"))), RuntimeError);
}

TEST(InterpreterExprTest, UnaryMinusOnNonNumberThrows) {
    EXPECT_THROW(runExpr(unary(TokenType::MINUS, "-", litStr("a"))), RuntimeError);
}
