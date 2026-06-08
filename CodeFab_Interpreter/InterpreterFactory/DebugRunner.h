#pragma once
#include "InterpreterRunner.h"
#include <string>

// Strategy 구현: 파일을 로드한 후 대화형 디버거를 실행
class DebugRunner : public InterpreterRunner {
public:
	explicit DebugRunner(std::string filePath);
	void run() override;

private:
	std::string filePath_;
};
