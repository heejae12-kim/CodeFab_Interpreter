#include "InterpreterFactory.h"
#include "ReplRunner.h"
#include "FileRunner.h"
#include "DebugRunner.h"
#include <stdexcept>

std::unique_ptr<InterpreterRunner> InterpreterFactory::create(RunMode mode, const std::string& filePath) {
	switch (mode) {
	case RunMode::REPL:
		return std::make_unique<ReplRunner>();
	case RunMode::FILE_RUN:
		return std::make_unique<FileRunner>(filePath);
	case RunMode::DEBUG:
		return std::make_unique<DebugRunner>(filePath);
	default:
		throw std::invalid_argument(
			"InterpreterFactory::create — unknown RunMode(" +
			std::to_string(static_cast<int>(mode)) + ")");
	}
}
