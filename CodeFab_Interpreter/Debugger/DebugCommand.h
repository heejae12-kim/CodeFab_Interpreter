#pragma once

class Debugger;

// Command 패턴: 디버그 명령의 추상 인터페이스
class DebugCommand {
public:
	virtual void execute(Debugger& debugger) = 0;
	virtual ~DebugCommand() = default;
};
