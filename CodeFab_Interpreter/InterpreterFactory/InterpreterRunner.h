#pragma once

#define interface struct

interface InterpreterRunner {
	virtual void run() = 0;
	virtual ~InterpreterRunner() = default;
};
