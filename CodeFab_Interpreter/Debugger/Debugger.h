#pragma once
#include <vector>
#include <set>
#include <string>
#include "../Stmt.h"
#include "../Interpreter.h"

// Command 패턴의 Invoker: 디버그 세션 상태를 관리하고 명령을 수신·실행
class Debugger {
public:
	Debugger(std::vector<StmtPtr>& stmts, Interpreter& interpreter, const std::string& source);
	void runDebugLoop();

	// Commands가 호출하는 액션 메서드
	void step();
	void next();
	void continueRun();
	void addBreakpoint(int line);
	void removeBreakpoint(int line);
	void addWatch(const std::string& var);
	void removeWatch(const std::string& var);
	void printWatches() const;
	void printInspect() const;
	bool isFinished() const;

private:
	std::vector<StmtPtr>& stmts_;
	Interpreter&          interpreter_;
	std::vector<std::string> sourceLines_;
	std::size_t           currentIdx_ = 0;
	std::set<int>         breakpoints_;
	std::vector<std::string> watches_;

	void showCurrentStmt() const;
	void showWatches() const;
	bool isBreakpoint(const Stmt& stmt) const;
	void skipFuncStmts();
	bool handleExitInput(const std::string& input);  // exit/quit 처리, true면 종료 요청

	bool exitRequested_     = false;
	bool continueAfterStep_ = false;
};
