#include "CheckerUnit.h"

void CheckerUnit::doChecker(vector<StmtPtr>& statements_tree_vector) {
	check_values_in_scopes_vector.push_back({});
	checkBlcok(statements_tree_vector);
	check_values_in_scopes_vector.pop_back();
}

void CheckerUnit::checkBlcok(const std::vector<StmtPtr>& statements_tree_vector) {
	for (const auto& statement_node : statements_tree_vector) {
		checkStatement(*statement_node);
	}
}

void CheckerUnit::checkStatement(Stmt& statements_node) {
	statements_node.accept(*this);
}

void CheckerUnit::checkExpression(Expr& expression_node) {
	expression_node.accept(*this);
}

ValuableValue CheckerUnit::visitLiteralExpr(LiteralExpr& expr) { 
	return {}; 
}

ValuableValue CheckerUnit::visitUnaryExpr(UnaryExpr& expr) {
	return {};
}

ValuableValue CheckerUnit::visitBinaryExpr(BinaryExpr& expr) {
	return {};
}

ValuableValue CheckerUnit::visitGroupingExpr(GroupingExpr& expr) {
	return {};
}

ValuableValue CheckerUnit::visitVariableExpr(VariableExpr& expr) {
	return {};
}

ValuableValue CheckerUnit::visitAssignExpr(AssignExpr& expr) {
	return {};
}


//// StmtVisitor
void CheckerUnit::visitPrintStmt(PrintStmt& stmt) {

}

void CheckerUnit::visitExprStmt(ExprStmt& stmt) {

}

void CheckerUnit::visitVarStmt(VarStmt& stmt) {
	auto declared_value_token = stmt.getName();
	auto& inner_most_scope = check_values_in_scopes_vector.back();
	if (inner_most_scope.count(declared_value_token.getLexme()))
		throw CheckerError("[line " 
			+ std::to_string(declared_value_token.getLine())
			+ "] Checker Error: Already a variable with this name in this scope.");

	if(stmt.getInitializer()) checkExpression(*stmt.getInitializer());

	inner_most_scope[declared_value_token.getLexme()] = false; // declared, not yet initialized
}

void CheckerUnit::visitBlockStmt(BlockStmt& stmt) {
	//
}

void CheckerUnit::visitIfStmt(IfStmt& stmt) {

}

void CheckerUnit::visitForStmt(ForStmt& stmt) {

}
