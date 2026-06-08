#include "DebugCommandParser.h"
#include "Commands/FlowCommands.h"
#include "Commands/BreakCommands.h"
#include "Commands/WatchCommands.h"
#include <sstream>

std::unique_ptr<DebugCommand> DebugCommandParser::parse(const std::string& input) {
	std::istringstream iss(input);
	std::string cmd;
	iss >> cmd;

	if (cmd == "step")     return std::make_unique<StepCommand>();
	if (cmd == "next")     return std::make_unique<NextCommand>();
	if (cmd == "continue") return std::make_unique<ContinueCommand>();
	if (cmd == "watches")  return std::make_unique<WatchesCommand>();
	if (cmd == "inspect")  return std::make_unique<InspectCommand>();
	if (cmd == "Breakpoints") return std::make_unique<BreakpointsCommand>();

	if (cmd == "break") {
		int line;
		if (iss >> line) return std::make_unique<BreakCommand>(line);
	}
	if (cmd == "remove") {
		int line;
		if (iss >> line) return std::make_unique<RemoveCommand>(line);
	}
	if (cmd == "watch") {
		std::string var;
		if (iss >> var) return std::make_unique<WatchCommand>(std::move(var));
	}
	if (cmd == "unwatch") {
		std::string var;
		if (iss >> var) return std::make_unique<UnwatchCommand>(std::move(var));
	}

	return nullptr;
}
