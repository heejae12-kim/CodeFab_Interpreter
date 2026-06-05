#include "Parser.h"

Parser::Parser(std::vector<Token> tokens_) :
	tokens_(std::move(tokens_)) {}

std::vector<StmtPtr> Parser::parse() {
	std::vector<StmtPtr> stmts_vector;
	while (!isAtEnd()) stmts_vector.push_back(declaration());
	return stmts_vector;
}

StmtPtr Parser::declaration() {
	if (match({ TokenType::FUNC })) return funcDeclaration();
	if (match({ TokenType::VAR }))  return varDeclaration();
	return statement();
}

StmtPtr Parser::funcDeclaration() {
	Token name = consume(TokenType::IDENTIFIER, "Expected function name.");
	consume(TokenType::LEFT_PAREN, "Expected '(' after function name.");
	std::vector<Token> params_vector;
	if (!check(TokenType::RIGHT_PAREN)) {
		do { params_vector.push_back(consume(TokenType::IDENTIFIER, "Expected parameter name.")); }
		while (match({ TokenType::COMMA }));
	}
	consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters.");
	consume(TokenType::LEFT_BRACE, "Expected '{' before function body.");
	std::vector<StmtPtr> body_vector = parseBlock("Expected '}' after function body.");
	return std::make_unique<FuncStmt>(std::move(name), std::move(params_vector), std::move(body_vector));
}

StmtPtr Parser::varDeclaration() {
	Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");

	ExprPtr p_init;
	if (match({ TokenType::EQUAL })) p_init = expression();
	consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");

	return std::make_unique<VarStmt>(std::move(name), std::move(p_init));
}

StmtPtr Parser::statement() {
	if (match({ TokenType::PRINT }))      return printStatement();
	if (match({ TokenType::IF }))         return ifStatement();
	if (match({ TokenType::FOR }))        return forStatement();
	if (match({ TokenType::LEFT_BRACE })) return blockStatement();
	if (match({ TokenType::RETURN }))     return returnStatement();
	return expressionStatement();
}

StmtPtr Parser::printStatement() {
	ExprPtr p_value = expression();
	consume(TokenType::SEMICOLON, "Expected ';' after value.");

	return std::make_unique<PrintStmt>(std::move(p_value));
}

StmtPtr Parser::ifStatement() {
	consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'.");
	ExprPtr p_cond = comparison();
	consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition.");
	StmtPtr p_thenBranch = statement();
	StmtPtr p_elseBranch;
	if (match({ TokenType::ELSE })) p_elseBranch = statement();
	return std::make_unique<IfStmt>(std::move(p_cond), std::move(p_thenBranch), std::move(p_elseBranch));
}

StmtPtr Parser::forStatement() {
	consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'.");

	StmtPtr p_init;
	if      (match({ TokenType::VAR }))    p_init = varDeclaration();
	else if (!check(TokenType::SEMICOLON)) p_init = expressionStatement();
	else                                   consume(TokenType::SEMICOLON, "");

	ExprPtr p_cond;
	if (!check(TokenType::SEMICOLON)) p_cond = comparison();
	consume(TokenType::SEMICOLON, "Expected ';' after for condition.");

	ExprPtr p_incr;
	if (!check(TokenType::RIGHT_PAREN)) p_incr = assignment();
	consume(TokenType::RIGHT_PAREN, "Expected ')' after for clauses.");

	StmtPtr p_body = statement();
	return std::make_unique<ForStmt>(std::move(p_init), std::move(p_cond), std::move(p_incr), std::move(p_body));
}

StmtPtr Parser::blockStatement() {
	return std::make_unique<BlockStmt>(parseBlock());
}

std::vector<StmtPtr> Parser::parseBlock(const std::string& closingMsg) {
	std::vector<StmtPtr> stmts_vector;
	while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
		stmts_vector.push_back(declaration());
	consume(TokenType::RIGHT_BRACE, closingMsg);
	return stmts_vector;
}

StmtPtr Parser::returnStatement() {
	Token keyword = previous();
	ExprPtr p_value;
	if (!check(TokenType::SEMICOLON)) p_value = expression();
	consume(TokenType::SEMICOLON, "Expected ';' after return value.");
	return std::make_unique<ReturnStmt>(std::move(keyword), std::move(p_value));
}

StmtPtr Parser::expressionStatement() {
	ExprPtr p_expr = expression();
	consume(TokenType::SEMICOLON, "Expected ';' after expression.");
	return std::make_unique<ExprStmt>(std::move(p_expr));
}

ExprPtr Parser::expression() {
	return assignment();
}

ExprPtr Parser::assignment() {
	ExprPtr p_expr = logicalOr();
	if (match({ TokenType::EQUAL })) {
		ExprPtr val = assignment();
		if (auto* var = dynamic_cast<VariableExpr*>(p_expr.get()))
			return std::make_unique<AssignExpr>(var->getName(), std::move(val));
		if (auto* idx = dynamic_cast<ArrIndexGetExpr*>(p_expr.get())) {
			Token   bracket = idx->getBracket();
			ExprPtr obj     = idx->takeObject();
			ExprPtr index   = idx->takeIndex();
			return std::make_unique<ArrIndexSetExpr>(
				std::move(obj), std::move(index), std::move(bracket), std::move(val));
		}
		throw ParseError("[line " + std::to_string(previous().getLine()) + "] Invalid assignment target.");
	}
	return p_expr;
}

ExprPtr Parser::parseBinary(std::function<ExprPtr()> next, std::vector<TokenType> ops) {
	ExprPtr p_expr = next();
	while (match(ops)) {
		Token   op  = previous();
		ExprPtr rhs = next();
		p_expr = std::make_unique<BinaryExpr>(std::move(p_expr), std::move(op), std::move(rhs));
	}
	return p_expr;
}

ExprPtr Parser::logicalOr()      { return parseBinary([this] { return logicalAnd(); },     { TokenType::OR_OP }); }
ExprPtr Parser::logicalAnd()     { return parseBinary([this] { return comparison(); },     { TokenType::AND_OP }); }
ExprPtr Parser::addition()       { return parseBinary([this] { return multiplication(); }, { TokenType::PLUS, TokenType::MINUS }); }
ExprPtr Parser::multiplication() { return parseBinary([this] { return unary(); },          { TokenType::STAR, TokenType::SLASH }); }

ExprPtr Parser::comparison() {
	return parseBinary([this] { return addition(); }, {
		TokenType::LESS,         TokenType::LESS_EQUAL,
		TokenType::GREATER,      TokenType::GREATER_EQUAL,
		TokenType::EQUAL_EQUAL,  TokenType::BANG_EQUAL
	});
}

ExprPtr Parser::unary() {
	if (match({ TokenType::MINUS, TokenType::BANG })) {
		Token op = previous();
		ExprPtr rhs = unary();
		return std::make_unique<UnaryExpr>(std::move(op), std::move(rhs));
	}
	return postfix();
}

ExprPtr Parser::postfix() {
	ExprPtr p_expr = primary();
	while (true) {
		if (match({ TokenType::LEFT_PAREN })) {
			p_expr = finishCall(std::move(p_expr));
		} else if (match({ TokenType::LEFT_BRACKET })) {
			Token   bracket = previous();
			ExprPtr p_index = expression();
			consume(TokenType::RIGHT_BRACKET, "Expected ']' after index.");
			p_expr = std::make_unique<ArrIndexGetExpr>(std::move(p_expr), std::move(p_index), std::move(bracket));
		} else {
			break;
		}
	}
	return p_expr;
}

ExprPtr Parser::finishCall(ExprPtr callee) {
	Token paren = previous();
	std::vector<ExprPtr> args_vector;
	if (!check(TokenType::RIGHT_PAREN)) {
		do { args_vector.push_back(expression()); } while (match({ TokenType::COMMA }));
	}
	consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments.");
	return std::make_unique<CallExpr>(std::move(callee), std::move(paren), std::move(args_vector));
}

ExprPtr Parser::primary() {
	if (match({ TokenType::NUMBER })) return std::make_unique<LiteralExpr>(previous().getLiteral());
	if (match({ TokenType::STRING })) return std::make_unique<LiteralExpr>(previous().getLiteral());
	if (check(TokenType::TRUE_KW)) {
		advance();
		return std::make_unique<LiteralExpr>(true);
	}
	if (check(TokenType::FALSE_KW)) {
		advance();
		return std::make_unique<LiteralExpr>(false);
	}
	if (check(TokenType::IDENTIFIER) || check(TokenType::ARRAY)) {
		Token tok = advance();
		return std::make_unique<VariableExpr>(std::move(tok));
	}
	if (match({ TokenType::LEFT_PAREN })) {
		ExprPtr p_expr = expression();
		consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
		return std::make_unique<GroupingExpr>(std::move(p_expr));
	}
	throw ParseError(makeError("Expected expression."));
}

std::string Parser::makeError(const std::string& msg) const {
	return "[line " + std::to_string(peek().getLine())
		+ "] Syntax Error at '" + peek().getLexme() + "': " + msg;
}

Token Parser::consume(TokenType type, const std::string& msg) {
	if (check(type)) return advance();
	throw ParseError(makeError(msg));
}

bool Parser::match(std::vector<TokenType> types) {
	for (auto type : types) {
		if (check(type)) { advance(); return true; }
	}
	return false;
}

bool Parser::check(TokenType t) const {
	return tokens_[current].getTokenType() == t;
}

bool Parser::isAtEnd() const {
	return tokens_[current].getTokenType() == TokenType::EOF_TOKEN;
}

Token& Parser::advance() {
	if (!isAtEnd()) ++current;
	return tokens_[current - 1];
}

const Token& Parser::peek() const {
	return tokens_[current];
}

Token& Parser::previous() {
	return tokens_[current - 1];
}
