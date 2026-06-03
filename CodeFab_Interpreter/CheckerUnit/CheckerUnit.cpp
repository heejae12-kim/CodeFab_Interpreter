#include "CheckerUnit.h"

void CheckerUnit::DoChecker(vector<StmtPtr>& statements_tree_vector) {
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

}

void CheckerUnit::visitBlockStmt(BlockStmt& stmt) {

}

void CheckerUnit::visitIfStmt(IfStmt& stmt) {

}

void CheckerUnit::visitForStmt(ForStmt& stmt) {

}
