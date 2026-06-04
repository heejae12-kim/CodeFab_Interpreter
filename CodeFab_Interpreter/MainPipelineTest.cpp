#include "gmock/gmock.h"
#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"
#include "Environment.h"
#include "TestHelpers.h"

using namespace testing;
using namespace tst;

// ── Lexer → Parser → Interpreter 전체 파이프라인 Fixture ──────────────────────
//
// 기존 InterpreterTest / InterpreterStmtTest 는 TestHelpers 로 AST 를 직접 조립한다.
// 이 Fixture 는 소스 문자열을 실제 Lexer/Parser 에 통과시켜 통합 동작을 검증한다.
//
// [현재 Parser 지원 범위]
//   - var 선언 : var name = NUMBER;
//   - 식 문     : IDENTIFIER;
//   → print / if / else / for / 블록 / 연산자는 Parser 확장 후 TC 추가 예정

class MainPipelineTestFixture : public Test {
protected:
    // source → tokens → stmts → interpret, stdout 캡처 반환
    std::string runSource(const std::string& src) {
        auto tokens  = Lexer(src).tokenize();                 // [Lexer] 소스 → 토큰 배열
        auto program = Parser(std::move(tokens)).parse();     // [Parser] 토큰 배열 → AST
        Interpreter interpreter;                              // [Interpreter] AST 실행
        CoutCapture capture;                                  // stdout 캡처 (tst::CoutCapture)
        interpreter.interpret(program);
        return capture.str();
    }

    // 예외 발생 여부만 검증할 때 사용 (stdout 캡처 불필요)
    void execSource(const std::string& src) {
        auto tokens  = Lexer(src).tokenize();
        auto program = Parser(std::move(tokens)).parse();
        Interpreter interpreter;
        interpreter.interpret(program);
    }
};

// ── TC-PIPE-001: 빈 입력 ───────────────────────────────────────────────────────
// [체크 파이프라인] Lexer → Parser → Interpreter
// Lexer: 빈 소스를 EOF_TOKEN 1개로 변환
// Parser: 빈 토큰 목록으로 빈 문장 배열 반환
// Interpreter: 빈 프로그램 실행 → 오류 없음
TEST_F(MainPipelineTestFixture, TC_PIPE_001_EmptySource) {
    EXPECT_NO_THROW(execSource(""));
}

// ── TC-PIPE-002: 공백·탭·개행만 ───────────────────────────────────────────────
// [체크 파이프라인] Lexer → Parser → Interpreter
// Lexer: 공백 문자를 모두 무시하고 EOF_TOKEN 반환
// Parser: 빈 토큰 목록 처리
// Interpreter: 빈 프로그램 실행 → 오류 없음
TEST_F(MainPipelineTestFixture, TC_PIPE_002_WhitespaceOnly) {
    EXPECT_NO_THROW(execSource("   \t\n  "));
}

// ── TC-PIPE-003: 주석만 있는 소스 ─────────────────────────────────────────────
// [체크 파이프라인] Lexer → Parser → Interpreter
// Lexer: '//' 이후를 줄 끝까지 무시하고 EOF_TOKEN 반환
// Parser: 빈 토큰 목록 처리
// Interpreter: 빈 프로그램 실행 → 오류 없음
TEST_F(MainPipelineTestFixture, TC_PIPE_003_CommentOnlySource) {
    EXPECT_NO_THROW(execSource("// this is a comment\n// another line"));
}

// ── TC-PIPE-004: var 단일 선언 ────────────────────────────────────────────────
// [체크 파이프라인] Lexer → Parser → Interpreter → Environment
// Lexer: "var x = 42;" → VAR, IDENTIFIER, EQUAL, NUMBER, SEMICOLON 토큰 생성
// Parser: VarStmt(x, LiteralExpr(42.0)) 생성
// Interpreter: Environment::define("x", 42.0) 호출 → 변수 저장
TEST_F(MainPipelineTestFixture, TC_PIPE_004_SingleVarDeclaration) {
    EXPECT_NO_THROW(execSource("var x = 42;"));
}

// ── TC-PIPE-005: var 복수 선언 ────────────────────────────────────────────────
// [체크 파이프라인] Lexer → Parser → Interpreter → Environment
// Lexer: 3개의 var 선언을 순서대로 토큰화
// Parser: VarStmt 3개를 순서대로 생성
// Interpreter: Environment::define을 3회 호출 → 변수 a, b, c 각각 저장
TEST_F(MainPipelineTestFixture, TC_PIPE_005_MultipleVarDeclarations) {
    EXPECT_NO_THROW(execSource("var a = 1; var b = 2; var c = 3;"));
}

// ── TC-PIPE-006: var 선언 후 식 참조 ─────────────────────────────────────────
// [체크 파이프라인] Lexer → Parser → Interpreter → Environment
// Lexer: "var x = 10; x;" → VAR, IDENTIFIER, EQUAL, NUMBER, SEMICOLON, IDENTIFIER, SEMICOLON
// Parser: VarStmt + ExprStmt(VariableExpr("x")) 생성
// Interpreter: VarStmt 실행 후 VariableExpr 평가 시 Environment::get("x") 호출
TEST_F(MainPipelineTestFixture, TC_PIPE_006_VarDeclareAndReference) {
    EXPECT_NO_THROW(execSource("var x = 10; x;"));
}

// ── TC-PIPE-007: 변수를 다른 변수 값으로 초기화 ──────────────────────────────
// [체크 파이프라인] Lexer → Parser → Interpreter → Environment
// Lexer: "var a = 1; var b = a;" 토큰화
// Parser: VarStmt(a, 1.0) + VarStmt(b, VariableExpr("a")) 생성
// Interpreter: a 저장 후 b 초기화 시 Environment::get("a") 호출 → b = 1.0
TEST_F(MainPipelineTestFixture, TC_PIPE_007_VarInitFromOtherVar) {
    EXPECT_NO_THROW(execSource("var a = 1; var b = a;"));
}

// ── TC-PIPE-008: 미정의 변수 참조 → RuntimeError ─────────────────────────────
// [체크 파이프라인] Lexer → Parser → Interpreter → Environment
// Lexer: "x;" 정상 토큰화 → Parser: ExprStmt(VariableExpr("x")) 생성
// Interpreter: VariableExpr 평가 시 Environment::get("x") 호출
// Environment: 미정의 변수이므로 RuntimeError throw
TEST_F(MainPipelineTestFixture, TC_PIPE_008_UndefinedVarThrows) {
    EXPECT_THROW(execSource("x;"), RuntimeError);
}

// ── TC-PIPE-009: 미정의 변수를 초기화에 사용 → RuntimeError ──────────────────
// [체크 파이프라인] Lexer → Parser → Interpreter → Environment
// Lexer: "var a = undefined_var;" 정상 토큰화
// Parser: VarStmt(a, VariableExpr("undefined_var")) 생성
// Interpreter: VarStmt 실행 중 initializer 평가 시 Environment::get("undefined_var") 호출
// Environment: "undefined_var" 미존재 → RuntimeError throw
TEST_F(MainPipelineTestFixture, TC_PIPE_009_UndefinedVarAsInitializerThrows) {
    EXPECT_THROW(execSource("var a = undefined_var;"), RuntimeError);
}

// ── TC-PIPE-010: 주석과 var 선언 혼재 ────────────────────────────────────────
// [체크 파이프라인] Lexer → Parser → Interpreter → Environment
// Lexer: 주석 라인을 건너뛰고 var 선언 토큰만 추출
// Parser: VarStmt 2개 생성 (주석은 토큰에 포함되지 않음)
// Interpreter: x, y 각각 Environment::define 호출
TEST_F(MainPipelineTestFixture, TC_PIPE_010_CommentAndVarMixed) {
    EXPECT_NO_THROW(execSource(
        "// declare x\n"
        "var x = 100;\n"
        "// declare y\n"
        "var y = 200;"
    ));
}

// ── TC-PIPE-011: 소수 리터럴 선언 ────────────────────────────────────────────
// [체크 파이프라인] Lexer → Parser → Interpreter → Environment
// Lexer: "3.14" → NUMBER 토큰, literal = 3.14(double)
// Parser: VarStmt(pi, LiteralExpr(3.14)) 생성
// Interpreter: Environment::define("pi", 3.14) 호출
TEST_F(MainPipelineTestFixture, TC_PIPE_011_FloatVarDeclaration) {
    EXPECT_NO_THROW(execSource("var pi = 3.14;"));
}
