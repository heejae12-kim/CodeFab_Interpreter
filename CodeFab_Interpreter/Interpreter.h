#pragma once
#include "Stmt.h"
#include "Environment.h"
#include <memory>
#include <vector>

// Stmt/Expr를 DFS 실행한다.
//   - Expr : 값을 평가하여 ValuableValue 반환
//   - Stmt : 동작 수행 (void), print로 출력
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
    bool        isFalse(const ValuableValue& v);   // nil/false 면 true (거짓 같은 값 판별)
    bool        isEqual(const ValuableValue& a, const ValuableValue& b);
    std::string stringify(const ValuableValue& v);
    void checkNumberOperand(const Token& op, const ValuableValue& operand);
    void checkNumberOperands(const Token& op, const ValuableValue& left, const ValuableValue& right);

    ValuableValue visitLiteralExpr(LiteralExpr& expr) override;
    ValuableValue visitUnaryExpr(UnaryExpr& expr) override;
    ValuableValue visitBinaryExpr(BinaryExpr& expr) override;
    ValuableValue visitGroupingExpr(GroupingExpr& expr) override;
    ValuableValue visitVariableExpr(VariableExpr& expr) override;
    ValuableValue visitAssignExpr(AssignExpr& expr) override;
    ValuableValue visitCallExpr(CallExpr& expr) override;
    ValuableValue visitIndexGetExpr(ArrIndexGetExpr& expr) override;
    ValuableValue visitIndexSetExpr(ArrIndexSetExpr& expr) override;

    void visitPrintStmt(PrintStmt& stmt) override;
    void visitExprStmt(ExprStmt& stmt) override;
    void visitVarStmt(VarStmt& stmt) override;
    void visitBlockStmt(BlockStmt& stmt) override;
    void visitIfStmt(IfStmt& stmt) override;
    void visitForStmt(ForStmt& stmt) override;

    void visitFuncStmt(FuncStmt& stmt) override;
    void visitReturnStmt(ReturnStmt& stmt) override;
};
