#pragma once
#include "Stmt.h"
#include <vector>
#include <initializer_list>

class Parser {
public:
	explicit Parser(std::vector<Token> tokens);
	std::vector<StmtPtr> parse();

private:
	std::vector<Token> tokens_;
	int current = 0;

	StmtPtr declaration();
	StmtPtr varDeclaration();
	StmtPtr statement();
	StmtPtr expressionStatement();

	ExprPtr expression();
	ExprPtr primary();

	bool match(std::initializer_list<TokenType> types);
	bool check(TokenType type);
	Token& advance();
	Token& peek();
	bool isAtEnd();
	Token& previous();
	Token consume(TokenType type);
};