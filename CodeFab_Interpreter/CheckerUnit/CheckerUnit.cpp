#include "CheckerUnit.h"

void CheckerUnit::doChecker(vector<StmtPtr>& statements_tree_vector) {
	addBeginBlockScope();
	checkBlock(statements_tree_vector);
	addEndBlockScope();
}

void CheckerUnit::checkBlock(const std::vector<StmtPtr>& statements_tree_vector) {
	for (const auto& statement_node : statements_tree_vector) {
		checkStatement(*statement_node);
	}
}

void CheckerUnit::checkStatement(Stmt& statements_node) {
	statements_node.accept(*this);
}

void CheckerUnit::checkExpression(ExprPtr& p_expression) {
	if (!p_expression) return;
	p_expression->accept(*this);
}

void CheckerUnit::checkParams(FuncStmt& stmt) {
	for (const auto& param : stmt.getParams()) {
		if (check_values_in_scopes_vector.back().count(param.getLexme()))
			throw CheckerError("[line " + std::to_string(param.getLine()) +
				"] Checker Error: Duplicate parameter name '" + param.getLexme() + "'.");
		declareValue(param);
		defineValue(param);
	}
}

void CheckerUnit::addBeginBlockScope() {
	check_values_in_scopes_vector.push_back({});
}

void CheckerUnit::addEndBlockScope() {
	check_values_in_scopes_vector.pop_back();
}

void CheckerUnit::declareValue(const Token& name) {
	if (check_values_in_scopes_vector.empty())
		return;

	auto& inner_most_scope = check_values_in_scopes_vector.back();
	if (inner_most_scope.count(name.getLexme()))
		throw CheckerError("[line " + std::to_string(name.getLine()) +
			"] Checker Error: Already a variable with this name in this scope.");
	inner_most_scope[name.getLexme()] = false;
}

void CheckerUnit::defineValue(const Token& name) {
	if (!check_values_in_scopes_vector.empty())
		check_values_in_scopes_vector.back()[name.getLexme()] = true;
}

void CheckerUnit::updateValueDistance(Expr& expr, const Token& name) {
	int distance = 0;
	for (auto scope_iter = check_values_in_scopes_vector.rbegin();
		scope_iter != check_values_in_scopes_vector.rend(); ++scope_iter, ++distance) {
		if (scope_iter->count(name.getLexme())) {
			auto p_variable_expr = dynamic_cast<VariableExpr*>(&expr);
			auto p_assign_expr = dynamic_cast<AssignExpr*>(&expr);
			if (p_variable_expr) p_variable_expr->setDistance(distance);
			else if (p_assign_expr) p_assign_expr->setDistance(distance);
			return;
		}
	}
}

#pragma region ExprVisitor

ValuableValue CheckerUnit::visitLiteralExpr(LiteralExpr& expr) {
	return nullptr;
}

ValuableValue CheckerUnit::visitUnaryExpr(UnaryExpr& expr) {
	checkExpression(expr.getRight());
	return nullptr;
}

ValuableValue CheckerUnit::visitBinaryExpr(BinaryExpr& expr) {
	checkExpression(expr.getLeft());
	checkExpression(expr.getRight());
	return nullptr;
}

ValuableValue CheckerUnit::visitGroupingExpr(GroupingExpr& expr) {
	// ( expression ) 
	checkExpression(expr.getExpression());
	return nullptr;
}

ValuableValue CheckerUnit::visitVariableExpr(VariableExpr& expr) {
	if (!check_values_in_scopes_vector.empty()) {
		auto& inner_most_scope = check_values_in_scopes_vector.back();
		auto iterator_value_in_scope = inner_most_scope.find(expr.getName().getLexme());

		if (iterator_value_in_scope != inner_most_scope.end() && !iterator_value_in_scope->second)
			throw CheckerError("[line " + std::to_string(expr.getName().getLine()) +
				"] Checker Error: Can't read local variable in its own initializer.");
	}
#ifdef USE_DISTANCE_OPTIMIZE
	updateValueDistance(expr, expr.getName());
#endif
	return nullptr;
}

ValuableValue CheckerUnit::visitAssignExpr(AssignExpr& expr) {
	checkExpression(expr.getValue());
#ifdef USE_DISTANCE_OPTIMIZE
	updateValueDistance(expr, expr.getName());
#endif
	return nullptr;
}

ValuableValue CheckerUnit::visitCallExpr(CallExpr& expr) {
	checkExpression(expr.getCallee());
	for (auto& arg : expr.getArguments()) checkExpression(arg);
	return nullptr;
}

#pragma endregion

#pragma region StmtVisitor

void CheckerUnit::visitPrintStmt(PrintStmt& stmt) {
	checkExpression(stmt.getExpression());
}

void CheckerUnit::visitExprStmt(ExprStmt& stmt) {
	checkExpression(stmt.getExpression());
}

void CheckerUnit::visitVarStmt(VarStmt& stmt) {
	declareValue(stmt.getName());
	if (stmt.getInitializer()) checkExpression(stmt.getInitializer());
	defineValue(stmt.getName());
}

void CheckerUnit::visitBlockStmt(BlockStmt& stmt) {
	addBeginBlockScope();
	checkBlock(stmt.getStatements());
	addEndBlockScope();
}

void CheckerUnit::visitIfStmt(IfStmt& stmt) {
	checkExpression(stmt.getCondition());
	checkStatement(*stmt.getThenBranch());

	if (stmt.getElseBranch())
		checkStatement(*stmt.getElseBranch());
}

void CheckerUnit::visitForStmt(ForStmt& stmt) {
	addBeginBlockScope();

	if (stmt.getInitializer()) checkStatement(*stmt.getInitializer());
	if (stmt.getCondition())   checkExpression(stmt.getCondition());
	if (stmt.getIncrement())   checkExpression(stmt.getIncrement());
	checkStatement(*stmt.getBody());

	addEndBlockScope();
}

void CheckerUnit::visitFuncStmt(FuncStmt& stmt) {
	declareValue(stmt.getName());
	defineValue(stmt.getName());
	bool backup_flag = is_in_function;
	is_in_function = true;
	addBeginBlockScope();
	checkParams(stmt);
	checkBlock(stmt.getBody());
	addEndBlockScope();
	is_in_function = backup_flag;
}

void CheckerUnit::visitReturnStmt(ReturnStmt& stmt) {
	if (!is_in_function)
		throw CheckerError("[line " + std::to_string(stmt.getKeyword().getLine()) +
			"] Checker Error: Can't return from top-level code.");
	checkExpression(stmt.getValue());  // null guard 내부 처리
}

#pragma endregion