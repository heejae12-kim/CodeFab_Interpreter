#pragma once
// 테스트 공용 헬퍼.
//  - Lexer/Parser 가 아직 없으므로 AST(Expr/Stmt)를 손으로 조립한다.
//  - Interpreter 는 print 를 std::cout 으로 출력하므로 stdout 을 가로채 검증한다.
#include "Expr.h"
#include "Stmt.h"
#include "Token.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace tst {

    // ── std::cout 캡처 (RAII) ───────────────────────────────────────────────
    class CoutCapture {
    public:
        CoutCapture() : old_(std::cout.rdbuf(buffer_.rdbuf())) {}
        ~CoutCapture() { std::cout.rdbuf(old_); }
        std::string str() const { return buffer_.str(); }
    private:
        std::ostringstream buffer_;
        std::streambuf*    old_;
    };

    // ── 값 추출 ─────────────────────────────────────────────────────────────
    inline double             asNum(const ValuableValue& v) { return std::get<double>(v); }
    inline const std::string& asStr(const ValuableValue& v) { return std::get<std::string>(v); }
    inline bool               asBool(const ValuableValue& v) { return std::get<bool>(v); }
    inline bool               isNil(const ValuableValue& v) {
        return std::holds_alternative<std::nullptr_t>(v);
    }

    // ── 토큰 ────────────────────────────────────────────────────────────────
    inline Token nameTok(const std::string& lexeme, int line = 1) {
        return Token(TokenType::IDENTIFIER, lexeme, nullptr, line);
    }
    inline Token opTok(TokenType type, const std::string& lexeme, int line = 1) {
        return Token(type, lexeme, nullptr, line);
    }

    // ── Expr 빌더 ───────────────────────────────────────────────────────────
    inline ExprPtr litNum(double v) { return std::make_unique<LiteralExpr>(v); }
    inline ExprPtr litStr(std::string s) { return std::make_unique<LiteralExpr>(std::move(s)); }
    inline ExprPtr litBool(bool b) { return std::make_unique<LiteralExpr>(b); }
    inline ExprPtr litNil() { return std::make_unique<LiteralExpr>(nullptr); }

    inline ExprPtr variable(const std::string& n, int line = 1) {
        return std::make_unique<VariableExpr>(nameTok(n, line));
    }
    inline ExprPtr assign(const std::string& n, ExprPtr value, int line = 1) {
        return std::make_unique<AssignExpr>(nameTok(n, line), std::move(value));
    }
    inline ExprPtr binary(ExprPtr left, TokenType op, const std::string& lex, ExprPtr right, int line = 1) {
        return std::make_unique<BinaryExpr>(std::move(left), opTok(op, lex, line), std::move(right));
    }
    inline ExprPtr unary(TokenType op, const std::string& lex, ExprPtr right, int line = 1) {
        return std::make_unique<UnaryExpr>(opTok(op, lex, line), std::move(right));
    }
    inline ExprPtr grouping(ExprPtr inner) {
        return std::make_unique<GroupingExpr>(std::move(inner));
    }

    // ── Stmt 빌더 ───────────────────────────────────────────────────────────
    inline StmtPtr printStmt(ExprPtr e) { return std::make_unique<PrintStmt>(std::move(e)); }
    inline StmtPtr exprStmt(ExprPtr e) { return std::make_unique<ExprStmt>(std::move(e)); }
    inline StmtPtr varStmt(const std::string& n, ExprPtr init, int line = 1) {
        return std::make_unique<VarStmt>(nameTok(n, line), std::move(init));
    }
    inline StmtPtr blockStmt(std::vector<StmtPtr> stmts) {
        return std::make_unique<BlockStmt>(std::move(stmts));
    }
    inline StmtPtr ifStmt(ExprPtr cond, StmtPtr thenB, StmtPtr elseB = nullptr) {
        return std::make_unique<IfStmt>(std::move(cond), std::move(thenB), std::move(elseB));
    }
    inline StmtPtr forStmt(StmtPtr init, ExprPtr cond, ExprPtr incr, StmtPtr body) {
        return std::make_unique<ForStmt>(std::move(init), std::move(cond), std::move(incr), std::move(body));
    }

    // 여러 StmtPtr 를 vector 로 묶는다 (move-only 이므로 가변 인자 + forward)
    template <typename... Stmts>
    inline std::vector<StmtPtr> stmts(Stmts&&... s) {
        std::vector<StmtPtr> v;
        (v.push_back(std::forward<Stmts>(s)), ...);
        return v;
    }

} // namespace tst
