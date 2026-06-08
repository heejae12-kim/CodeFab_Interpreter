#pragma once
#include "../DebugCommand.h"
#include <string>

// 변수 감시 커맨드

class WatchCommand : public DebugCommand {
public:
	explicit WatchCommand(std::string var) : var_(std::move(var)) {}
	void execute(Debugger& debugger) override;
private:
	std::string var_;
};

class UnwatchCommand : public DebugCommand {
public:
	explicit UnwatchCommand(std::string var) : var_(std::move(var)) {}
	void execute(Debugger& debugger) override;
private:
	std::string var_;
};

class WatchesCommand : public DebugCommand {
public:
	void execute(Debugger& debugger) override;
};

class InspectCommand : public DebugCommand {
public:
	void execute(Debugger& debugger) override;
};
