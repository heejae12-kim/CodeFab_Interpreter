#pragma once
#include "../DebugCommand.h"

// 브레이크포인트 관리 커맨드

class BreakCommand : public DebugCommand {
public:
	explicit BreakCommand(int line) : line_(line) {}
	void execute(Debugger& debugger) override;
private:
	int line_;
};

class RemoveCommand : public DebugCommand {
public:
	explicit RemoveCommand(int line) : line_(line) {}
	void execute(Debugger& debugger) override;
private:
	int line_;
};
