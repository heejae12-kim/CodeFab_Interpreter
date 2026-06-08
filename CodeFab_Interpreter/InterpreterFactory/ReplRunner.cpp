#include "ReplRunner.h"
#include "../Lexer.h"
#include "../Parser.h"
#include "../CheckerUnit/CheckerError.h"
#include "../Environment.h"
#include <iostream>
#include <string>

void ReplRunner::run() {
	std::cout << "CodeFab REPL  (exit / quit to quit)\n";
	std::cout << "> ";

	std::string source;
	int braceDepth = 0;
	std::string line;

	while (std::getline(std::cin, line)) {
		if (braceDepth == 0 && (line == "exit" || line == "quit"))
			break;

		source += line + "\n";

		for (char c : line) {
			if (c == '{') ++braceDepth;
			else if (c == '}') --braceDepth;
		}

		if (braceDepth <= 0) {
			braceDepth = 0;
			if (!source.empty())
				runSource(source);
			source.clear();
			std::cout << "> ";
		} else {
			std::cout << "... ";
		}
	}

	std::cout << "Bye!\n";
}

void ReplRunner::runSource(const std::string& source) {
	try {
		Lexer lexer(source);
		auto tokens = lexer.tokenize();

		Parser parser(std::move(tokens));
		auto stmts = parser.parse();

		checker_.doCheckerRepl(stmts);
		interpreter_.interpret(stmts);
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
