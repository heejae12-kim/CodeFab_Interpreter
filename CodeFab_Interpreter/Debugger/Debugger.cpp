#include "Debugger.h"
#include "DebugCommandParser.h"
#include "../Environment.h"
#include <iostream>
#include <sstream>
#include <algorithm>

static std::string lineTag(int line) {
	return "[line : " + std::to_string(line) + "]";
}

static std::string stmtTypeName(const Stmt& stmt) {
	if (dynamic_cast<const PrintStmt*> (&stmt)) return "PrintStmt";
	if (dynamic_cast<const VarStmt*>   (&stmt)) return "VarStmt";
	if (dynamic_cast<const ExprStmt*>  (&stmt)) return "ExprStmt";
	if (dynamic_cast<const IfStmt*>    (&stmt)) return "IfStmt";
	if (dynamic_cast<const ForStmt*>   (&stmt)) return "ForStmt";
	if (dynamic_cast<const BlockStmt*> (&stmt)) return "BlockStmt";
	if (dynamic_cast<const FuncStmt*>  (&stmt)) return "FuncStmt";
	if (dynamic_cast<const ReturnStmt*>(&stmt)) return "ReturnStmt";
	return "Stmt";
}

Debugger::Debugger(std::vector<StmtPtr>& stmts, Interpreter& interpreter, const std::string& source)
	: stmts_(stmts), interpreter_(interpreter) {
	std::istringstream ss(source);
	std::string line;
	while (std::getline(ss, line))
		sourceLines_.push_back(line);
}

void Debugger::runDebugLoop() {
	std::cout << "CodeFab Debugger\n"
	          << "Commands: step | next | continue | break <n> | remove <n> | Breakpoints | watch <var> | unwatch <var> | watches | inspect | exit\n"
	          << "──────────────────────────────────────────\n";

	if (stmts_.empty()) {
		std::cout << "[Debugger] No statements to execute.\n";
		return;
	}

	skipFuncStmts();
	showCurrentStmt();

	std::string line;
	while (!isFinished() && !exitRequested_) {
		std::cout << "(debug) ";
		if (!std::getline(std::cin, line)) break;
		if (line == "exit" || line == "quit") break;

		auto cmd = DebugCommandParser::parse(line);
		if (cmd)
			cmd->execute(*this);
		else
			std::cout << "[Debugger] Unknown command: '" << line << "'\n";

		if (!isFinished() && !exitRequested_) showCurrentStmt();
	}

	if (isFinished())
		std::cout << "[Debugger] Program finished.\n";
}

void Debugger::step() {
	if (isFinished()) {
		std::cout << "[Debugger] Program already finished.\n";
		return;
	}

	// hook: depth > 0 (함수·블록 내부)의 각 stmt마다 멈추고 사용자 입력 대기
	interpreter_.setStmtHook([&](Stmt& stmt, int depth) {
		if (depth == 0) return;  // 최상위 stmt는 이미 showCurrentStmt()에서 표시

		int line = stmt.getLine();
		std::cout << lineTag(line) << ">> ";
		if (line > 0 && line <= static_cast<int>(sourceLines_.size()))
			std::cout << sourceLines_[line - 1] << "\n";
		else
			std::cout << stmtTypeName(stmt) << "\n";
		showWatches();

		std::string input;
		while (true) {
			std::cout << "(debug) ";
			if (!std::getline(std::cin, input)) {
				interpreter_.clearStmtHook();
				return;
			}
			if (handleExitInput(input)) return;
			if (input == "step") break;  // 다음 서브 stmt로 이동
			if (input == "next") {
				interpreter_.clearStmtHook();
				return;
			}
			if (input == "continue") {
				continueAfterStep_ = true;
				interpreter_.clearStmtHook();
				return;
			}
			// watch / inspect 등 부가 커맨드 처리
			auto cmd = DebugCommandParser::parse(input);
			if (cmd) cmd->execute(*this);
			else std::cout << "[Debugger] In step mode: step | next | continue | watches | inspect | exit\n";
		}
	});

	skipFuncStmts();
	if (isFinished()) return;

	try {
		interpreter_.executeSingleStmt(*stmts_[currentIdx_]);
	} catch (const RuntimeError& e) {
		std::cerr << "[Runtime Error] " << e.what() << "\n";
	}
	interpreter_.clearStmtHook();
	++currentIdx_;
	skipFuncStmts();
	showWatches();
	if (continueAfterStep_) {
		continueAfterStep_ = false;
		continueRun();
	}
}

void Debugger::next() {
	// loop/block 내부는 진입해 stmt마다 멈추지만, 함수 내부는 진입하지 않음
	if (isFinished()) {
		std::cout << "[Debugger] Program already finished.\n";
		return;
	}
	skipFuncStmts();
	if (isFinished()) return;

	interpreter_.setStmtHook([&](Stmt& stmt, int depth) {
		if (depth == 0) return;                        // 최상위 stmt는 건너뜀
		if (interpreter_.getFunctionDepth() > 0) return;  // 함수 내부는 건너뜀

		int line = stmt.getLine();
		std::cout << lineTag(line) << ">> ";
		if (line > 0 && line <= static_cast<int>(sourceLines_.size()))
			std::cout << sourceLines_[line - 1] << "\n";
		else
			std::cout << stmtTypeName(stmt) << "\n";
		showWatches();

		std::string input;
		while (true) {
			std::cout << "(debug) ";
			if (!std::getline(std::cin, input)) {
				interpreter_.clearStmtHook();
				return;
			}
			if (handleExitInput(input)) return;
			if (input == "next" || input == "step") break;
			if (input == "continue") {
				continueAfterStep_ = true;
				interpreter_.clearStmtHook();
				return;
			}
			auto cmd = DebugCommandParser::parse(input);
			if (cmd) cmd->execute(*this);
			else std::cout << "[Debugger] In next mode: next | step | continue | watches | inspect | exit\n";
		}
	});

	try {
		interpreter_.executeSingleStmt(*stmts_[currentIdx_]);
	} catch (const RuntimeError& e) {
		std::cerr << "[Runtime Error] " << e.what() << "\n";
	}
	interpreter_.clearStmtHook();
	++currentIdx_;
	skipFuncStmts();
	showWatches();
	if (continueAfterStep_) {
		continueAfterStep_ = false;
		continueRun();
	}
}

void Debugger::continueRun() {
	// 블록·루프·함수 내부(depth > 0) stmt에서도 브레이크포인트를 검사한다.
	bool subBreakHit = false;
	interpreter_.setStmtHook([&](Stmt& stmt, int depth) {
		if (depth == 0) return;
		if (!breakpoints_.count(stmt.getLine())) return;

		int line = stmt.getLine();
		std::cout << lineTag(line) << " Breakpoint hit\n";
		if (line > 0 && line <= static_cast<int>(sourceLines_.size()))
			std::cout << lineTag(line) << ">> " << sourceLines_[line - 1] << "\n";
		showWatches();

		std::string input;
		while (true) {
			std::cout << "(debug) ";
			if (!std::getline(std::cin, input)) {
				interpreter_.clearStmtHook();
				subBreakHit = true;
				return;
			}
			if (handleExitInput(input)) { subBreakHit = true; return; }
			if (input == "continue") return;  // 훅 유지, 다음 브레이크포인트까지 계속
			if (input == "step" || input == "next") {
				// 현재 top-level stmt 실행 완료 후 runDebugLoop로 복귀
				interpreter_.clearStmtHook();
				subBreakHit = true;
				return;
			}
			auto cmd = DebugCommandParser::parse(input);
			if (cmd) cmd->execute(*this);
			else std::cout << "[Debugger] Breakpoint. Commands: continue | step | next | watches | inspect | exit\n";
		}
	});

	bool isFirst = true;
	while (!isFinished() && !exitRequested_) {
		if (!isFirst && isBreakpoint(*stmts_[currentIdx_])) {
			int bpLine = stmts_[currentIdx_]->getLine();
			std::cout << lineTag(bpLine) << " Breakpoint hit\n";
			showWatches();
			interpreter_.clearStmtHook();
			return;
		}
		isFirst = false;
		try {
			interpreter_.executeSingleStmt(*stmts_[currentIdx_]);
		} catch (const RuntimeError& e) {
			std::cerr << "[Runtime Error] " << e.what() << "\n";
			interpreter_.clearStmtHook();
			++currentIdx_;
			return;
		}
		++currentIdx_;

		if (subBreakHit) {
			interpreter_.clearStmtHook();
			return;
		}
	}
	interpreter_.clearStmtHook();
	showWatches();
}

void Debugger::addBreakpoint(int line) {
	breakpoints_.insert(line);
	std::cout << "[Debugger] Breakpoint set at line " << line << "\n";
}

void Debugger::removeBreakpoint(int line) {
	if (breakpoints_.erase(line))
		std::cout << "[Debugger] Breakpoint removed at line " << line << "\n";
	else
		std::cout << "[Debugger] No breakpoint at line " << line << "\n";
}

void Debugger::printBreakpoints() const {
	if (breakpoints_.empty()) {
		std::cout << "[Debugger] No breakpoints set.\n";
		return;
	}
	std::cout << "[Debugger] Breakpoints:\n";
	for (int bp : breakpoints_)
		std::cout << "  " << lineTag(bp) << "\n";
}

void Debugger::addWatch(const std::string& var) {
	watches_.push_back(var);
	std::cout << "[Debugger] Watching '" << var << "' = " << interpreter_.getVarAsString(var) << "\n";
}

void Debugger::removeWatch(const std::string& var) {
	auto it = std::find(watches_.begin(), watches_.end(), var);
	if (it != watches_.end()) {
		watches_.erase(it);
		std::cout << "[Debugger] Stopped watching '" << var << "'\n";
	} else {
		std::cout << "[Debugger] '" << var << "' is not being watched.\n";
	}
}

void Debugger::printWatches() const {
	if (watches_.empty())
		std::cout << "[Debugger] No variables being watched.\n";
	else
		showWatches();
}

void Debugger::printInspect() const {
	std::cout << "── Inspect ──────────────────────────────\n";
	std::cout << "  Position : stmt[" << currentIdx_ << " / " << stmts_.size() << "]";
	if (!isFinished())
		std::cout << "  " << lineTag(stmts_[currentIdx_]->getLine())
		          << "  " << stmtTypeName(*stmts_[currentIdx_]);
	std::cout << "\n";

	std::cout << "  Breakpoints:";
	if (breakpoints_.empty()) std::cout << " (none)";
	for (int bp : breakpoints_) std::cout << " " << bp;
	std::cout << "\n";

	std::cout << "  Watches:\n";
	if (watches_.empty()) {
		std::cout << "    (none)\n";
	} else {
		for (const auto& v : watches_)
			std::cout << "    " << v << " = " << interpreter_.getVarAsString(v) << "\n";
	}

	// 전역 변수 출력
	auto globalVars = interpreter_.getGlobalVars();
	std::cout << "  [Global]\n";
	if (globalVars.empty()) {
		std::cout << "    (none)\n";
	} else {
		for (const auto& [name, val] : globalVars)
			std::cout << "    " << name << " = " << val << "\n";
	}

	// 로컬 변수 출력 (전역 스코프가 아닐 때만)
	auto localVars = interpreter_.getLocalVars();
	if (!localVars.empty()) {
		std::cout << "  [Local]\n";
		for (const auto& [name, val] : localVars)
			std::cout << "    " << name << " = " << val << "\n";
	}

	std::cout << "─────────────────────────────────────────\n";
}

bool Debugger::isFinished() const {
	return currentIdx_ >= stmts_.size();
}

void Debugger::showCurrentStmt() const {
	if (isFinished()) return;
	const auto& stmt = *stmts_[currentIdx_];
	int line = stmt.getLine();

	std::cout << lineTag(line) << ">> ";
	if (line > 0 && line <= static_cast<int>(sourceLines_.size()))
		std::cout << sourceLines_[line - 1] << "\n";
	else
		std::cout << stmtTypeName(stmt) << "\n";
}

void Debugger::showWatches() const {
	for (const auto& v : watches_)
		std::cout << "  watch  " << v << " = " << interpreter_.getVarAsString(v) << "\n";
}

bool Debugger::isBreakpoint(const Stmt& stmt) const {
	return breakpoints_.count(stmt.getLine()) > 0;
}

bool Debugger::handleExitInput(const std::string& input) {
	if (input == "exit" || input == "quit") {
		exitRequested_ = true;
		interpreter_.clearStmtHook();
		return true;
	}
	return false;
}

void Debugger::skipFuncStmts() {
	while (!isFinished() && dynamic_cast<FuncStmt*>(stmts_[currentIdx_].get())) {
		const auto& stmt = *stmts_[currentIdx_];
		int line = stmt.getLine();
		std::cout << lineTag(line) << ">> ";
		if (line > 0 && line <= static_cast<int>(sourceLines_.size()))
			std::cout << sourceLines_[line - 1] << "\n";
		else
			std::cout << "FuncStmt\n";

		try {
			interpreter_.executeSingleStmt(*stmts_[currentIdx_]);
		} catch (const RuntimeError& e) {
			std::cerr << "[Runtime Error] " << e.what() << "\n";
		}
		++currentIdx_;
	}
}
