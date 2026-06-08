#pragma once
#include "InterpreterRunner.h"
#include "../Interpreter.h"
#include "../CheckerUnit/CheckerUnit.h"
#include <string>

class ReplRunner : public InterpreterRunner {
public:
	void run() override;

private:
	Interpreter  interpreter_;
	CheckerUnit  checker_;

	void runSource(const std::string& source);
};
