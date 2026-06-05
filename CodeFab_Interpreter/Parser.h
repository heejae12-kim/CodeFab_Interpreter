#pragma once
#include <vector>
#include "Stmt.h"
#include <functional>
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
	StmtPtr funcDeclaration();
	StmtPtr statement();
	StmtPtr expressionStatement();
	StmtPtr printStatement();
	StmtPtr ifStatement();
	StmtPtr forStatement();
	StmtPtr blockStatement();
	StmtPtr returnStatement();

	ExprPtr              parseBinary(std::function<ExprPtr()> next, std::vector<TokenType> ops);
	std::vector<StmtPtr> parseBlock(const std::string& closingMsg = "Expected '}' after block.");

	ExprPtr expression();
	ExprPtr assignment();
	ExprPtr logicalOr();
	ExprPtr logicalAnd();
	ExprPtr comparison();
	ExprPtr addition();
	ExprPtr multiplication();
	ExprPtr unary();
	ExprPtr postfix();
	ExprPtr finishCall(ExprPtr callee);
	ExprPtr primary();

	bool         match(std::vector<TokenType> types);
	bool         check(TokenType type) const;
	bool         isAtEnd() const;
	const Token& peek() const;
	Token&       advance();
	Token&       previous();
	Token        consume(TokenType type, const std::string& msg);
};
