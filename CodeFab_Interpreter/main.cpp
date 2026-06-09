#include "gmock/gmock.h"
#include "InterpreterFactory/InterpreterFactory.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <clocale>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
	// 콘솔 출력/입력 코드페이지를 UTF-8로 맞춘다 (유니코드 구분선·한글 출력이 ????로 깨지지 않도록).
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif
	// CRT가 UTF-8 바이트를 시스템 ANSI로 오변환하지 않도록 로케일도 UTF-8로 설정.
	std::setlocale(LC_ALL, ".UTF-8");

	if (argc == 1) {
		auto runner = InterpreterFactory::create(RunMode::REPL);
		runner->run();
		return 0;
	}

	std::string option = argv[1];

	std::transform(option.begin(), option.end(), option.begin(), [](unsigned char c) {
		return std::tolower(c);
		});

	if (option == "unit") {
		testing::InitGoogleMock();
		return RUN_ALL_TESTS();
	} else if (option == "run" && argc >= 3) {
		auto runner = InterpreterFactory::create(RunMode::FILE_RUN, argv[2]);
		runner->run();
	} else if (option == "debug" && argc >= 3) {
		auto runner = InterpreterFactory::create(RunMode::DEBUG, argv[2]);
		runner->run();
	} else {
		std::cerr << "Usage:\n"
		          << "  (no args)          : REPL mode\n"
		          << "  unit               : run unit tests\n"
		          << "  run   <file>       : execute file\n"
		          << "  debug <file>       : debug file\n";
		return 1;
	}

	return 0;
}
