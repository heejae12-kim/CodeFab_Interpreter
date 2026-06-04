#pragma once
#include "Stmt.h"
#include "Environment.h"
#include <memory>
#include <vector>

// Executor Unit (PDF Chapter 04) — 트리-워크 인터프리터.
// 문법 트리(Stmt/Expr 조립체)를 DFS 로 재귀 평가하여 실제로 실행한다.
//   - Expr 방문 : 값을 평가하여 ValuableValue 반환
//   - Stmt 방문 : 동작 수행 (void), print 는 std::cout 으로 출력
class Interpreter : public ExprVisitor, public StmtVisitor {
public:
    Interpreter();
    void interpret(const std::vector<StmtPtr>& statements);
    void executeBlock(const std::vector<StmtPtr>& stmts, std::shared_ptr<Environment> env);

private:
    std::shared_ptr<Environment> globalEnv;
    std::shared_ptr<Environment> currentEnv;

    ValuableValue evaluate(Expr& expr);
    void          execute(Stmt& stmt);

    bool        isTruthy(const ValuableValue& v);
    bool        isEqual(const ValuableValue& a, const ValuableValue& b);
    std::string stringify(const ValuableValue& v);
    void checkNumberOperand(const Token& op, const ValuableValue& operand);
    void checkNumberOperands(const Token& op, const ValuableValue& left, const ValuableValue& right);

    // ExprVisitor
    ValuableValue visitLiteralExpr(LiteralExpr& expr) override;
    ValuableValue visitUnaryExpr(UnaryExpr& expr) override;
    ValuableValue visitBinaryExpr(BinaryExpr& expr) override;
    ValuableValue visitGroupingExpr(GroupingExpr& expr) override;
    ValuableValue visitVariableExpr(VariableExpr& expr) override;
    ValuableValue visitAssignExpr(AssignExpr& expr) override;

    // StmtVisitor
    void visitPrintStmt(PrintStmt& stmt) override;
    void visitExprStmt(ExprStmt& stmt) override;
    void visitVarStmt(VarStmt& stmt) override;
    void visitBlockStmt(BlockStmt& stmt) override;
    void visitIfStmt(IfStmt& stmt) override;
    void visitForStmt(ForStmt& stmt) override;
};
