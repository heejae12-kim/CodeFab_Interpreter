// 함수/배열 Executor 단위 테스트 (파서가 아직 노드를 만들지 않으므로 AST를 직접 조립).
#include "gmock/gmock.h"
#include "Interpreter.h"
#include "Environment.h"   // RuntimeError
#include "TestHelpers.h"

using namespace tst;

namespace {
    Token parenTok()  { return Token(TokenType::RIGHT_PAREN, ")", nullptr, 1); }
    Token bracketTok(){ return Token(TokenType::LEFT_BRACKET, "[", nullptr, 1); }
    Token returnTok() { return Token(TokenType::RETURN, "return", nullptr, 1); }
    Token idTok(const std::string& n) { return Token(TokenType::IDENTIFIER, n, nullptr, 1); }

    template <typename... E>
    std::vector<ExprPtr> args(E&&... e) {
        std::vector<ExprPtr> v;
        (v.push_back(std::forward<E>(e)), ...);
        return v;
    }

    ExprPtr callExpr(const std::string& name, std::vector<ExprPtr> a) {
        return std::make_unique<CallExpr>(variable(name), parenTok(), std::move(a));
    }
    ExprPtr arrayCreate(ExprPtr size) {
        std::vector<ExprPtr> a; a.push_back(std::move(size));
        return std::make_unique<CallExpr>(variable("Array"), parenTok(), std::move(a));
    }
    ExprPtr indexGet(ExprPtr obj, ExprPtr idx) {
        return std::make_unique<ArrIndexGetExpr>(std::move(obj), std::move(idx), bracketTok());
    }
    ExprPtr indexSet(ExprPtr obj, ExprPtr idx, ExprPtr val) {
        return std::make_unique<ArrIndexSetExpr>(std::move(obj), std::move(idx), bracketTok(), std::move(val));
    }
    StmtPtr returnStmt(ExprPtr value) {
        return std::make_unique<ReturnStmt>(returnTok(), std::move(value));
    }
    StmtPtr funcStmt(const std::string& name, std::vector<std::string> params, std::vector<StmtPtr> body) {
        std::vector<Token> ps;
        for (auto& p : params) ps.push_back(idTok(p));
        return std::make_unique<FuncStmt>(idTok(name), std::move(ps), std::move(body));
    }

    std::string run(std::vector<StmtPtr> program) {
        Interpreter interpreter;
        CoutCapture capture;
        interpreter.interpret(program);
        return capture.str();
    }
}

// ── 함수 ─────────────────────────────────────────────────────────────────────
TEST(ExecutorFuncTest, BasicCallReturnsValue) {
    auto program = stmts(
        funcStmt("add", { "a", "b" },
                 stmts(returnStmt(binary(variable("a"), TokenType::PLUS, "+", variable("b"))))),
        printStmt(callExpr("add", args(litNum(3.0), litNum(7.0)))));
    EXPECT_EQ(run(std::move(program)), "10\n");
}

TEST(ExecutorFuncTest, ReturnWithoutValueIsNil) {
    auto program = stmts(
        funcStmt("f", {}, stmts(returnStmt(nullptr))),
        printStmt(callExpr("f", args())));
    EXPECT_EQ(run(std::move(program)), "nil\n");
}

TEST(ExecutorFuncTest, Recursion) {
    auto body = stmts(
        ifStmt(binary(variable("n"), TokenType::LESS_EQUAL, "<=", litNum(1.0)),
               returnStmt(litNum(1.0))),
        returnStmt(binary(variable("n"), TokenType::STAR, "*",
                   callExpr("fact", args(binary(variable("n"), TokenType::MINUS, "-", litNum(1.0)))))));
    auto program = stmts(funcStmt("fact", { "n" }, std::move(body)),
                         printStmt(callExpr("fact", args(litNum(5.0)))));
    EXPECT_EQ(run(std::move(program)), "120\n");
}

TEST(ExecutorFuncTest, ArityMismatchThrows) {
    auto program = stmts(
        funcStmt("add", { "a", "b" }, stmts(returnStmt(litNum(0.0)))),
        exprStmt(callExpr("add", args(litNum(1.0)))));
    EXPECT_THROW(run(std::move(program)), RuntimeError);
}

TEST(ExecutorFuncTest, CallingNonFunctionThrows) {
    auto program = stmts(
        varStmt("x", litStr("hi")),
        exprStmt(callExpr("x", args())));
    EXPECT_THROW(run(std::move(program)), RuntimeError);
}

// ── 정적 배열 ────────────────────────────────────────────────────────────────
TEST(ExecutorArrayTest, CreateWriteRead) {
    auto program = stmts(
        varStmt("arr", arrayCreate(litNum(3.0))),
        exprStmt(indexSet(variable("arr"), litNum(0.0), litNum(10.0))),
        printStmt(indexGet(variable("arr"), litNum(0.0))));
    EXPECT_EQ(run(std::move(program)), "10\n");
}

TEST(ExecutorArrayTest, DefaultElementIsNil) {
    auto program = stmts(
        varStmt("arr", arrayCreate(litNum(2.0))),
        printStmt(indexGet(variable("arr"), litNum(0.0))));
    EXPECT_EQ(run(std::move(program)), "nil\n");
}

TEST(ExecutorArrayTest, PrintWholeArray) {
    auto program = stmts(
        varStmt("arr", arrayCreate(litNum(3.0))),
        exprStmt(indexSet(variable("arr"), litNum(0.0), litNum(1.0))),
        exprStmt(indexSet(variable("arr"), litNum(1.0), litNum(2.0))),
        exprStmt(indexSet(variable("arr"), litNum(2.0), litNum(3.0))),
        printStmt(variable("arr")));
    EXPECT_EQ(run(std::move(program)), "[1, 2, 3]\n");
}

TEST(ExecutorArrayTest, IndexOutOfRangeThrows) {
    auto program = stmts(
        varStmt("arr", arrayCreate(litNum(3.0))),
        printStmt(indexGet(variable("arr"), litNum(5.0))));
    EXPECT_THROW(run(std::move(program)), RuntimeError);
}

TEST(ExecutorArrayTest, NonNumberIndexThrows) {
    auto program = stmts(
        varStmt("arr", arrayCreate(litNum(3.0))),
        printStmt(indexGet(variable("arr"), litStr("hello"))));
    EXPECT_THROW(run(std::move(program)), RuntimeError);
}

TEST(ExecutorArrayTest, IndexingNonArrayThrows) {
    auto program = stmts(
        varStmt("x", litNum(10.0)),
        printStmt(indexGet(variable("x"), litNum(0.0))));
    EXPECT_THROW(run(std::move(program)), RuntimeError);
}

TEST(ExecutorArrayTest, NonNumberSizeThrows) {
    auto program = stmts(varStmt("brr", arrayCreate(litStr("hi"))));
    EXPECT_THROW(run(std::move(program)), RuntimeError);
}
