#pragma once
#include <memory>
#include <string>
#include "InterpreterRunner.h"

enum class RunMode { REPL, FILE_RUN };

class InterpreterFactory {
public:
	static std::unique_ptr<InterpreterRunner> create(RunMode mode, const std::string& filePath = "");
};
