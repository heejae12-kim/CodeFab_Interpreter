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
    if (tokens_[current].getTokenType() == TokenType::EQUAL) {
        current++;              // EQUAL
        init = expression();
    }
    consume(TokenType::SEMICOLON);
    return std::make_unique<VarStmt>(std::move(name), std::move(init));
}

StmtPtr Parser::statement() {
    if (match({ TokenType::PRINT }))      return printStatement();
    if (match({ TokenType::IF }))         return ifStatement();
    if (match({ TokenType::LEFT_BRACE })) return blockStatement();
    if (tokens_[current].getTokenType() == TokenType::FOR) {
        current++;  // FOR
        current++;  // LEFT_PAREN
        Token initName = tokens_[current++];
        current++;
        ValuableValue initVal = tokens_[current++].getLiteral();
        current++;

        StmtPtr init = std::make_unique<ExprStmt>(
            std::make_unique<AssignExpr>(
                std::move(initName),
                std::make_unique<LiteralExpr>(std::move(initVal))
            )
        );
        Token condLeft = tokens_[current++];
        Token condOp = tokens_[current++];
        ValuableValue condRight = tokens_[current++].getLiteral();  // NUMBER
        current++;

        ExprPtr cond = std::make_unique<BinaryExpr>(
            std::make_unique<VariableExpr>(std::move(condLeft)),
            std::move(condOp),
            std::make_unique<LiteralExpr>(std::move(condRight))
        );
        Token incrName = tokens_[current++];
        current++;
        Token incrLeft = tokens_[current++];
        Token incrOp = tokens_[current++];
        ValuableValue incrRight = tokens_[current++].getLiteral();  // NUMBER
        current++;

        ExprPtr incr = std::make_unique<AssignExpr>(
            std::move(incrName),
            std::make_unique<BinaryExpr>(
                std::make_unique<VariableExpr>(std::move(incrLeft)),
                std::move(incrOp),
                std::make_unique<LiteralExpr>(std::move(incrRight))
            )
        );
        StmtPtr body = statement();  // LEFT_BRACE → blockStatement()
        return std::make_unique<ForStmt>(std::move(init), std::move(cond), std::move(incr), std::move(body));
    }
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
    return addition();
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
    if (check(TokenType::NUMBER)) {
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

Token   Parser::consume(TokenType type)
{
    if (check(type)) return advance();
    return peek();
}
