#pragma once
#include "InterpreterRunner.h"
#include "../Interpreter.h"
#include "../CheckerUnit/CheckerUnit.h"
#include "../Stmt.h"
#include <string>
#include <vector>

class ReplRunner : public InterpreterRunner {
public:
	void run() override;

private:
	Interpreter  interpreter_;
	CheckerUnit  checker_;
	// FunctionObject가 FuncStmt&를 참조하므로 AST 수명을 REPL 세션 내내 유지한다.
	std::vector<std::vector<StmtPtr>> stmtHistory_;

	void runSource(const std::string& source);
};
