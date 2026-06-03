#pragma once
#include <vector>
#include <string>
#include "../Stmt.h"

using std::vector;
using std::string;

class CheckerUnit: 
	public ExprVisitor
	,public StmtVisitor
{
public:
	void DoChecker(vector<StmtPtr>& statements_tree_vector);

private:
#pragma region ExprVisitor

    ValuableValue visitLiteralExpr(LiteralExpr& expr) override;
    ValuableValue visitUnaryExpr(UnaryExpr& expr) override;
    ValuableValue visitBinaryExpr(BinaryExpr& expr) override;
    ValuableValue visitGroupingExpr(GroupingExpr& expr) override;
    ValuableValue visitVariableExpr(VariableExpr& expr) override;
    ValuableValue visitAssignExpr(AssignExpr& expr) override;

#pragma endregion

#pragma region StmtVisitor

    void visitPrintStmt(PrintStmt& stmt) override;
    void visitExprStmt(ExprStmt& stmt) override;
    void visitVarStmt(VarStmt& stmt) override;
    void visitBlockStmt(BlockStmt& stmt) override;
    void visitIfStmt(IfStmt& stmt) override;
    void visitForStmt(ForStmt& stmt) override;

#pragma endregion

};
