#include "gmock/gmock.h"
#include "Interpreter.h"
#include "TestHelpers.h"

using namespace tst;

namespace {
    std::string run(std::vector<StmtPtr> program) {
        Interpreter interpreter;
        CoutCapture capture;
        interpreter.interpret(program);
        return capture.str();
    }
}

TEST(InterpreterStmtTest, VariableDeclareAndUse) {
    // var a = 10; print a + 5;  → 15
    EXPECT_EQ(run(stmts(
        varStmt("a", litNum(10.0)),
        printStmt(binary(variable("a"), TokenType::PLUS, "+", litNum(5.0))))), "15\n");
}

TEST(InterpreterStmtTest, VarWithoutInitializerIsNil) {
    EXPECT_EQ(run(stmts(
        varStmt("a", nullptr),
        printStmt(variable("a")))), "nil\n");
}

TEST(InterpreterStmtTest, ReadUndefinedVariableThrows) {
    EXPECT_THROW(run(stmts(printStmt(variable("x")))), RuntimeError);
}

TEST(InterpreterStmtTest, AssignUpdatesVariable) {
    // var a = 1; a = 5; print a;  → 5
    EXPECT_EQ(run(stmts(
        varStmt("a", litNum(1.0)),
        exprStmt(assign("a", litNum(5.0))),
        printStmt(variable("a")))), "5\n");
}

TEST(InterpreterStmtTest, AssignToUndefinedThrows) {
    EXPECT_THROW(run(stmts(exprStmt(assign("x", litNum(1.0))))), RuntimeError);
}

TEST(InterpreterStmtTest, PrintStringBoolNil) {
    EXPECT_EQ(run(stmts(
        printStmt(litStr("hello")),
        printStmt(litBool(true)),
        printStmt(litNil()))), "hello\ntrue\nnil\n");
}

TEST(InterpreterStmtTest, BlockScopeDoesNotLeak) {
    EXPECT_THROW(run(stmts(
        blockStmt(stmts(varStmt("b", litNum(1.0)))),
        exprStmt(variable("b")))), RuntimeError);
}

TEST(InterpreterStmtTest, BlockReadsEnclosingVariable) {
    EXPECT_EQ(run(stmts(
        varStmt("a", litNum(10.0)),
        blockStmt(stmts(printStmt(variable("a")))))), "10\n");
}

TEST(InterpreterStmtTest, BlockShadowsThenRestoresOuter) {
    // var a = 1; { var a = 2; print a; } print a;  → 2, 1
    EXPECT_EQ(run(stmts(
        varStmt("a", litNum(1.0)),
        blockStmt(stmts(
            varStmt("a", litNum(2.0)),
            printStmt(variable("a")))),
        printStmt(variable("a")))), "2\n1\n");
}

TEST(InterpreterStmtTest, IfExecutesThenWhenTrue) {
    EXPECT_EQ(run(stmts(ifStmt(litBool(true),
        printStmt(litNum(1.0)),
        printStmt(litNum(2.0))))), "1\n");
}

TEST(InterpreterStmtTest, IfExecutesElseWhenFalse) {
    EXPECT_EQ(run(stmts(ifStmt(litBool(false),
        printStmt(litNum(1.0)),
        printStmt(litNum(2.0))))), "2\n");
}

TEST(InterpreterStmtTest, IfWithoutElseSkipsWhenFalse) {
    EXPECT_EQ(run(stmts(ifStmt(litBool(false), printStmt(litNum(1.0))))), "");
}

TEST(InterpreterStmtTest, ForLoopRepeatsBody) {
    // for (var i = 0; i < 3; i = i + 1) { print "#"; }  → "#\n#\n#\n"
    EXPECT_EQ(run(stmts(forStmt(
        varStmt("i", litNum(0.0)),
        binary(variable("i"), TokenType::LESS, "<", litNum(3.0)),
        assign("i", binary(variable("i"), TokenType::PLUS, "+", litNum(1.0))),
        blockStmt(stmts(printStmt(litStr("#"))))))), "#\n#\n#\n");
}

TEST(InterpreterStmtTest, ForLoopVariableScopedToLoop) {
    EXPECT_THROW(run(stmts(
        forStmt(
            varStmt("i", litNum(0.0)),
            binary(variable("i"), TokenType::LESS, "<", litNum(1.0)),
            assign("i", binary(variable("i"), TokenType::PLUS, "+", litNum(1.0))),
            blockStmt(stmts())),
        exprStmt(variable("i")))), RuntimeError);
}
