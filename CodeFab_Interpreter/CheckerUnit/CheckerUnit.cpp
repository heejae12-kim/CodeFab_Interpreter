#include "CheckerUnit.h"

void CheckerUnit::doChecker(vector<StmtPtr>& statements_tree_vector) {
	addBeginBlockScope();
	checkBlcok(statements_tree_vector);
	addEndBlockScope();
}

void CheckerUnit::addBeginBlockScope() {
	check_values_in_scopes_vector.push_back({});
}

void CheckerUnit::addEndBlockScope() {
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
	return nullptr;
}

ValuableValue CheckerUnit::visitUnaryExpr(UnaryExpr& expr) {
	checkExpression(*expr.getRight());
	return nullptr;
}

ValuableValue CheckerUnit::visitBinaryExpr(BinaryExpr& expr) {
	return {};
}

ValuableValue CheckerUnit::visitGroupingExpr(GroupingExpr& expr) {
	// ( expression ) 
	checkExpression(*expr.getExpression());
	return nullptr;
}

ValuableValue CheckerUnit::visitVariableExpr(VariableExpr& expr) {
	return nullptr;
}

ValuableValue CheckerUnit::visitAssignExpr(AssignExpr& expr) {
	return {};
}


//// StmtVisitor
void CheckerUnit::visitPrintStmt(PrintStmt& stmt) {
	checkExpression(*stmt.getExpression());
}

void CheckerUnit::visitExprStmt(ExprStmt& stmt) {
	checkExpression(*stmt.getExpression());
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
	addBeginBlockScope();
	checkBlcok(stmt.getStatements());
	addEndBlockScope();
}

void CheckerUnit::visitIfStmt(IfStmt& stmt) {

}

void CheckerUnit::visitForStmt(ForStmt& stmt) {

}
