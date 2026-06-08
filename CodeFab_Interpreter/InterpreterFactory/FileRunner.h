#pragma once
#include "InterpreterRunner.h"
#include <string>

class FileRunner : public InterpreterRunner {
public:
	explicit FileRunner(std::string filePath);
	void run() override;

private:
	std::string filePath_;
	void runSource(const std::string& source);
};
