#pragma once
#include "Stmt.h"
#include <vector>
#include <initializer_list>
#include <stdexcept>

class ParseError : public std::runtime_error {
public:
	explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

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
	StmtPtr printStatement();
	StmtPtr ifStatement();
	StmtPtr forStatement();
	StmtPtr blockStatement();

	ExprPtr expression();
	ExprPtr assignment();
	ExprPtr comparison();
	ExprPtr addition();
	ExprPtr multiplication();
	ExprPtr unary();
	ExprPtr primary();

	bool match(std::initializer_list<TokenType> types);
	bool check(TokenType type);
	Token& advance();
	Token& peek();
	bool isAtEnd();
	Token& previous();
	Token consume(TokenType type, const std::string& msg);
};
