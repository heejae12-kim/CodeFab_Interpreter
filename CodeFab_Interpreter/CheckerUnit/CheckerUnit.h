#pragma once
#include <vector>
#include <unordered_map>

#include "./CheckerError.h"
#include "../Stmt.h"

#define USE_DISTANCE_OPTIMIZE

using std::vector;
using std::string;
using std::unordered_map;

class CheckerUnit :
	public ExprVisitor,
	public StmtVisitor
{
public:
	void doChecker(vector<StmtPtr>& statements_tree_vector);

private:
	void checkBlock(const std::vector<StmtPtr>& statements_tree_vector);
	void checkStatement(Stmt& statements_node);
	void checkExpression(ExprPtr& p_expression);
	void addBeginBlockScope();
	void addEndBlockScope();
	void declareValue(const Token& name);
	void defineValue(const Token& name);

	void updateValueDistance(Expr& expr, const Token& name);

#pragma region ExprVisitor

	ValuableValue visitLiteralExpr(LiteralExpr& expr) override;
	ValuableValue visitUnaryExpr(UnaryExpr& expr) override;
	ValuableValue visitBinaryExpr(BinaryExpr& expr) override;
	ValuableValue visitGroupingExpr(GroupingExpr& expr) override;
	ValuableValue visitVariableExpr(VariableExpr& expr) override;
	ValuableValue visitAssignExpr(AssignExpr& expr) override;
	ValuableValue visitCallExpr(CallExpr& expr) override;
	ValuableValue visitIndexGetExpr(ArrIndexGetExpr& expr) override { return {}; };
	ValuableValue visitIndexSetExpr(ArrIndexSetExpr& expr) override { return {}; };

#pragma endregion

#pragma region StmtVisitor

	void visitPrintStmt(PrintStmt& stmt) override;
	void visitExprStmt(ExprStmt& stmt) override;
	void visitVarStmt(VarStmt& stmt) override;
	void visitBlockStmt(BlockStmt& stmt) override;
	void visitIfStmt(IfStmt& stmt) override;
	void visitForStmt(ForStmt& stmt) override;
	void visitFuncStmt(FuncStmt& stmt) override {};
	void visitReturnStmt(ReturnStmt& stmt) override {};

#pragma endregion

private:
	vector<unordered_map<string, bool>> check_values_in_scopes_vector;
};
