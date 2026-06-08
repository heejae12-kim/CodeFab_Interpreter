#pragma once
#include <memory>
#include <string>
#include "DebugCommand.h"

// 사용자 입력 문자열을 DebugCommand 객체로 변환 (Command 생성 팩토리)
class DebugCommandParser {
public:
	static std::unique_ptr<DebugCommand> parse(const std::string& input);
};
