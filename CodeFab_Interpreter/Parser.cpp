#include "Parser.h"

Parser::Parser(std::vector<Token> tokens_) :
    tokens_(std::move(tokens_)) {}

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> stmts;
    while (!isAtEnd()) stmts.push_back(declaration());
    return stmts;
}

StmtPtr Parser::declaration() {
    if (match({ TokenType::VAR })) return
        varDeclaration();
    return statement();

}

StmtPtr Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");

    ExprPtr init;
    if (match({ TokenType::EQUAL })) init = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");

    return std::make_unique<VarStmt>(std::move(name), std::move(init));
}
StmtPtr Parser::statement() {
    return expressionStatement();
}

StmtPtr Parser::expressionStatement() {
    ExprPtr expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    return std::make_unique<ExprStmt>(std::move(expr));
}

ExprPtr Parser::expression() {
    return assignment();
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
