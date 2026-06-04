#include "Parser.h"

Parser::Parser(std::vector<Token> tokens_) :
    tokens_(std::move(tokens_)) {}

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> stmts_vector;
    while (!isAtEnd()) stmts_vector.push_back(declaration());
    return stmts_vector;
}

StmtPtr Parser::declaration() {
    if (match({ TokenType::VAR })) return varDeclaration();
    return statement();

}

StmtPtr Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER);
    ExprPtr init;
    if (match({ TokenType::EQUAL })) init = expression();
    consume(TokenType::SEMICOLON);
    return std::make_unique<VarStmt>(std::move(name), std::move(init));
}

StmtPtr Parser::statement() {
    if (match({ TokenType::PRINT }))      return printStatement();
    if (match({ TokenType::IF }))         return ifStatement();
    if (match({ TokenType::FOR }))        return forStatement();
    if (match({ TokenType::LEFT_BRACE })) return blockStatement();
    return expressionStatement();
}

StmtPtr Parser::printStatement() {
    ExprPtr value = expression();
    consume(TokenType::SEMICOLON);
    return std::make_unique<PrintStmt>(std::move(value));
}

StmtPtr Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN);
    ExprPtr cond = comparison();
    consume(TokenType::RIGHT_PAREN);
    StmtPtr thenBranch = statement();
    StmtPtr elseBranch;
    if (match({ TokenType::ELSE })) elseBranch = statement();
    return std::make_unique<IfStmt>(std::move(cond), std::move(thenBranch), std::move(elseBranch));
}

StmtPtr Parser::forStatement() {
    consume(TokenType::LEFT_PAREN);
    StmtPtr init = expressionStatement();
    ExprPtr cond = comparison();
    consume(TokenType::SEMICOLON);
    ExprPtr incr = assignment();
    consume(TokenType::RIGHT_PAREN);
    StmtPtr body = statement();
    return std::make_unique<ForStmt>(std::move(init), std::move(cond), std::move(incr), std::move(body));
}

StmtPtr Parser::blockStatement() {
    std::vector<StmtPtr> stmts;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
        stmts.push_back(declaration());
    consume(TokenType::RIGHT_BRACE);
    return std::make_unique<BlockStmt>(std::move(stmts));
}

StmtPtr Parser::expressionStatement() {
    ExprPtr expr = expression();
    consume(TokenType::SEMICOLON);
    return std::make_unique<ExprStmt>(std::move(expr));
}

ExprPtr Parser::expression() {
    return assignment();
}

ExprPtr Parser::assignment() {
    ExprPtr expr = comparison();
    if (match({ TokenType::EQUAL })) {
        ExprPtr val = assignment();
        if (auto* var = dynamic_cast<VariableExpr*>(expr.get()))
            return std::make_unique<AssignExpr>(var->getName(), std::move(val));
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
    ExprPtr expr = primary();
    while (match({ TokenType::STAR, TokenType::SLASH }))
    {
        Token op = previous();
        ExprPtr rhs = primary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(rhs));
    }
    return expr;
}

ExprPtr Parser::primary() {
    if (check(TokenType::NUMBER) || check(TokenType::STRING)) {
        Token tok = advance();
        return std::make_unique<LiteralExpr>(tok.getLiteral());
    }
    if (check(TokenType::IDENTIFIER)) {
        Token tok = advance();
        return std::make_unique<VariableExpr>(std::move(tok));
    }
    return nullptr;
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

Token Parser::consume(TokenType type)
{
    if (check(type)) return advance();
    return peek();
}

