#include "gmock/gmock.h"
#include "InterpreterFactory/InterpreterFactory.h"
#include <string>
#include <iostream>

int main(int argc, char* argv[]) {
	if (argc == 1) {
		auto runner = InterpreterFactory::create(RunMode::REPL);
		runner->run();
		return 0;
	}

	std::string option = argv[1];

	if (option == "unit") {
		testing::InitGoogleMock(&argc, argv);
		return RUN_ALL_TESTS();
	} else if (option == "run" && argc >= 3) {
		auto runner = InterpreterFactory::create(RunMode::FILE_RUN, argv[2]);
		runner->run();
	} else {
		std::cerr << "Usage:\n"
		          << "  (no args)          : REPL mode\n"
		          << "  unit               : run unit tests\n"
		          << "  run <file>         : execute file\n";
		return 1;
	}

	return 0;
}
