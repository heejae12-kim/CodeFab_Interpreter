#include "Parser.h"

Parser::Parser(std::vector<Token> tokens_) :
    tokens_(std::move(tokens_)) {}

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> stmts_vector;
    while (!isAtEnd()) stmts_vector.push_back(declaration());
    return stmts_vector;
}

StmtPtr Parser::declaration() {
    if (match({ TokenType::VAR })) return
        varDeclaration();
    return statement();

}

StmtPtr Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER);
    consume(TokenType::EQUAL);
    ExprPtr p_init = expression();
    consume(TokenType::SEMICOLON);
    return std::make_unique<VarStmt>(std::move(name),
        std::move(p_init));
}
StmtPtr Parser::statement() {
    if (match({ TokenType::PRINT })) 
        return printStatement();
    return expressionStatement();
}

StmtPtr Parser::printStatement() {
    ExprPtr value = expression();
    consume(TokenType::SEMICOLON);
    return std::make_unique<PrintStmt>(std::move(value));
}


StmtPtr Parser::expressionStatement() {
    ExprPtr expr = expression();
    consume(TokenType::SEMICOLON);
    return std::make_unique<ExprStmt>(std::move(expr));
}

ExprPtr Parser::expression() {
    return addition();
}

ExprPtr Parser::addition() {
    ExprPtr expr = primary();

    // 하드코딩: * / 를 multiplication() 없이 직접 처리
    while (tokens_[current].getTokenType() == TokenType::STAR || tokens_[current].getTokenType() == TokenType::SLASH) {
        Token op = tokens_[current++];
        ExprPtr rhs = primary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(rhs));
    }

    while (match({ TokenType::PLUS, TokenType::MINUS }))
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
    return
        tokens_[current].getTokenType() == t;
}
bool Parser::isAtEnd()  {
    return
        tokens_[current].getTokenType() == TokenType::EOF_TOKEN;
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
