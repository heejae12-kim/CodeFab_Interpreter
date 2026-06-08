#pragma once
#include "../DebugCommand.h"

// 실행 흐름 제어 커맨드

class StepCommand : public DebugCommand {
public:
	void execute(Debugger& debugger) override;
};

// next: 현재 stmt를 실행하되 블록·함수 내부로 진입하지 않음
// (최상위 stmt 단위 실행이므로 step과 동일 효과)
class NextCommand : public DebugCommand {
public:
	void execute(Debugger& debugger) override;
};

class ContinueCommand : public DebugCommand {
public:
	void execute(Debugger& debugger) override;
};
