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
    std::vector<StmtPtr> p_body;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
        p_body.push_back(declaration());
    consume(TokenType::RIGHT_BRACE, "Expected '}' after function body.");
    return std::make_unique<FuncStmt>(std::move(name), std::move(params_vector), std::move(p_body));
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
    ExprPtr value = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after value.");

    return std::make_unique<PrintStmt>(std::move(value));
}

StmtPtr Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'.");
    ExprPtr cond = comparison();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition.");
    StmtPtr thenBranch = statement();
    StmtPtr elseBranch;
    if (match({ TokenType::ELSE })) elseBranch = statement();
    return std::make_unique<IfStmt>(std::move(cond), std::move(thenBranch), std::move(elseBranch));
}

StmtPtr Parser::forStatement() {
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'.");
    StmtPtr p_init;
    if (match({ TokenType::VAR })) p_init = varDeclaration();
    else p_init = expressionStatement();

    ExprPtr cond = comparison();
    consume(TokenType::SEMICOLON, "Expected ';' after for condition.");
    ExprPtr incr = assignment();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after for clauses.");
    StmtPtr body = statement();
    return std::make_unique<ForStmt>(std::move(p_init), std::move(cond), std::move(incr), std::move(body));
}

StmtPtr Parser::returnStatement() {
    Token keyword = previous();
    ExprPtr value;
    if (!check(TokenType::SEMICOLON)) value = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after return value.");
    return std::make_unique<ReturnStmt>(std::move(keyword), std::move(value));
}

StmtPtr Parser::blockStatement() {
    std::vector<StmtPtr> stmts_vector;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
        stmts_vector.push_back(declaration());
    consume(TokenType::RIGHT_BRACE, "Expected '}' after block.");
    return std::make_unique<BlockStmt>(std::move(stmts_vector));
}

StmtPtr Parser::expressionStatement() {
    ExprPtr expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    return std::make_unique<ExprStmt>(std::move(expr));
}

ExprPtr Parser::expression() {
    return assignment();
}

ExprPtr Parser::assignment() {
    ExprPtr expr = logicalOr();
    if (match({ TokenType::EQUAL })) {
        ExprPtr val = assignment();
        if (auto* var = dynamic_cast<VariableExpr*>(expr.get()))
            return std::make_unique<AssignExpr>(var->getName(), std::move(val));
        throw ParseError("[line " + std::to_string(previous().getLine()) + "] Invalid assignment target.");
    }
    return expr;
}

ExprPtr Parser::logicalOr() {
    ExprPtr expr = logicalAnd();
    while (match({ TokenType::OR_OP })) {
        Token op = previous();
        ExprPtr rhs = logicalAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(rhs));
    }
    return expr;
}

ExprPtr Parser::logicalAnd() {
    ExprPtr expr = comparison();
    while (match({ TokenType::AND_OP })) {
        Token op = previous();
        ExprPtr rhs = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(rhs));
    }
    return expr;
}

ExprPtr Parser::comparison() {
    ExprPtr expr = addition();
    while (match({ TokenType::LESS, TokenType::LESS_EQUAL,
                   TokenType::GREATER, TokenType::GREATER_EQUAL,
                   TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL })) {
        Token op = previous();
        ExprPtr rhs = addition();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(rhs));
    }
    return expr;
}

ExprPtr Parser::addition() {
    ExprPtr expr = multiplication();
    while (match({ TokenType::PLUS, TokenType::MINUS }))
    {
        Token op = previous();
        ExprPtr rhs = multiplication();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(rhs));
    }
    return expr;
}

ExprPtr Parser::multiplication() {
    ExprPtr expr = unary();
    while (match({ TokenType::STAR, TokenType::SLASH }))
    {
        Token op = previous();
        ExprPtr rhs = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(rhs));
    }
    return expr;
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
    ExprPtr expr = primary();
    while (true) {
        if (match({ TokenType::LEFT_PAREN })) {
            expr = finishCall(std::move(expr));
        } else if (match({ TokenType::LEFT_BRACKET })) {
            Token   bracket = previous();
            ExprPtr index   = expression();
            consume(TokenType::RIGHT_BRACKET, "Expected ']' after index.");
            expr = std::make_unique<ArrIndexGetExpr>(std::move(expr), std::move(index), std::move(bracket));
        } else {
            break;
        }
    }
    return expr;
}

ExprPtr Parser::finishCall(ExprPtr callee) {
    Token paren = previous();
    std::vector<ExprPtr> args;
    if (!check(TokenType::RIGHT_PAREN)) {
        do { args.push_back(expression()); } while (match({ TokenType::COMMA }));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments.");
    return std::make_unique<CallExpr>(std::move(callee), std::move(paren), std::move(args));
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
    if (check(TokenType::IDENTIFIER)) {
        Token tok = advance();
        return std::make_unique<VariableExpr>(std::move(tok));
    }
    if (match({ TokenType::LEFT_PAREN })) {
            ExprPtr expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }
    throw ParseError("[line " + std::to_string(peek().getLine()) + "] Syntax Error at '" + peek().getLexme() + "': Expected expression.");
}

bool Parser::check(TokenType t)  {
    return tokens_[current].getTokenType() == t;
}

bool Parser::isAtEnd()  {
    return tokens_[current].getTokenType() == TokenType::EOF_TOKEN;
}

Token& Parser::advance() {
    if (!isAtEnd()) ++current;
    return tokens_[current - 1];
}

Token& Parser::peek() { 
    return tokens_[current]; 
}

Token& Parser::previous() {
    return tokens_[current -1];
}

bool Parser::match(std::initializer_list<TokenType>
    types) {
    for (auto t : types) {
        if (check(t)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& msg) {
    if (check(type)) return advance();
    throw ParseError("[line " + std::to_string(peek().getLine()) + "] Syntax Error at '" + peek().getLexme() + "': " + msg);
}
