#include "FileRunner.h"
#include "../Lexer.h"
#include "../Parser.h"
#include "../CheckerUnit/CheckerUnit.h"
#include "../CheckerUnit/CheckerError.h"
#include "../Interpreter.h"
#include "../Environment.h"
#include <fstream>
#include <sstream>
#include <iostream>

FileRunner::FileRunner(std::string filePath) : filePath_(std::move(filePath)) {}

void FileRunner::run() {
	std::ifstream file(filePath_);
	if (!file.is_open()) {
		std::cerr << "[Error] Cannot open file: " << filePath_ << "\n";
		return;
	}
	std::ostringstream buf;
	buf << file.rdbuf();
	runSource(buf.str());
}

void FileRunner::runSource(const std::string& source) {
	try {
		Lexer lexer(source);
		auto tokens = lexer.tokenize();

		Parser parser(std::move(tokens));
		auto stmts = parser.parse();

		CheckerUnit checker;
		checker.doChecker(stmts);

		Interpreter interpreter;
		interpreter.interpret(stmts);
	}
	catch (const LexError& e) {
		std::cerr << "[Lex Error] "     << e.what() << "\n";
	}
	catch (const ParseError& e) {
		std::cerr << "[Parse Error] "   << e.what() << "\n";
	}
	catch (const CheckerError& e) {
		std::cerr << "[Checker Error] " << e.what() << "\n";
	}
	catch (const RuntimeError& e) {
		std::cerr << "[Runtime Error] " << e.what() << "\n";
	}
}
