#include "gmock/gmock.h"
#include "Lexer.h"

using namespace testing;

// ── 헬퍼 ──────────────────────────────────────────────────────────────────────

class LexerTestFixture : public Test {
protected:
    std::vector<Token> tokenize(const std::string& src) {
        return Lexer(src).tokenize();
    }

    // EOF를 제외한 토큰 타입 목록 반환
    std::vector<TokenType> types(const std::string& src) {
        auto tokens = tokenize(src);
        std::vector<TokenType> result;
        for (const auto& t : tokens) {
            if (t.getTokenType() != TokenType::EOF_TOKEN)
                result.push_back(t.getTokenType());
        }
        return result;
    }
};

// ── TC-LEX-001: 빈 입력 ───────────────────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_001_EmptyInput) {
    auto tokens = tokenize("");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].getTokenType(), TokenType::EOF_TOKEN);
}

// ── TC-LEX-002: 공백·탭·개행 무시 ─────────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_002_WhitespaceIgnored) {
    auto tokens = tokenize("   \t\n  ");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].getTokenType(), TokenType::EOF_TOKEN);
}

// ── TC-LEX-003: 단일 문자 토큰 ───────────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_003_SingleCharTokens) {
    auto result = types("( ) { } ; + - * /");
    EXPECT_THAT(result, ElementsAre(
        TokenType::LEFT_PAREN,
        TokenType::RIGHT_PAREN,
        TokenType::LEFT_BRACE,
        TokenType::RIGHT_BRACE,
        TokenType::SEMICOLON,
        TokenType::PLUS,
        TokenType::MINUS,
        TokenType::STAR,
        TokenType::SLASH
    ));
}

// ── TC-LEX-004: 비교·대입 연산자 ─────────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_004_ComparisonAndAssignOperators) {
    auto result = types("< <= > >= == != =");
    EXPECT_THAT(result, ElementsAre(
        TokenType::LESS,
        TokenType::LESS_EQUAL,
        TokenType::GREATER,
        TokenType::GREATER_EQUAL,
        TokenType::EQUAL_EQUAL,
        TokenType::BANG_EQUAL,
        TokenType::EQUAL
    ));
}

// ── TC-LEX-005: 정수 NUMBER 리터럴 ───────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_005_IntegerNumber) {
    auto tokens = tokenize("42");
    ASSERT_GE(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].getTokenType(), TokenType::NUMBER);
    EXPECT_EQ(tokens[0].getLexme(), "42");
    EXPECT_DOUBLE_EQ(std::get<double>(tokens[0].getLiteral()), 42.0);
}

// ── TC-LEX-006: 소수 NUMBER 리터럴 ───────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_006_FloatNumber) {
    auto tokens = tokenize("3.14");
    ASSERT_GE(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].getTokenType(), TokenType::NUMBER);
    EXPECT_EQ(tokens[0].getLexme(), "3.14");
    EXPECT_DOUBLE_EQ(std::get<double>(tokens[0].getLiteral()), 3.14);
}

// ── TC-LEX-007: STRING 리터럴 ─────────────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_007_StringLiteral) {
    auto tokens = tokenize("\"hello world\"");
    ASSERT_GE(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].getTokenType(), TokenType::STRING);
    // lexeme은 따옴표 포함, literal 값은 따옴표 제외
    EXPECT_EQ(std::get<std::string>(tokens[0].getLiteral()), "hello world");
}

// ── TC-LEX-008: 빈 문자열 ─────────────────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_008_EmptyString) {
    auto tokens = tokenize("\"\"");
    ASSERT_GE(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].getTokenType(), TokenType::STRING);
    EXPECT_EQ(std::get<std::string>(tokens[0].getLiteral()), "");
}

// ── TC-LEX-009: 키워드 ────────────────────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_009_Keywords) {
    auto result = types("var if else for print true false");
    EXPECT_THAT(result, ElementsAre(
        TokenType::VAR,
        TokenType::IF,
        TokenType::ELSE,
        TokenType::FOR,
        TokenType::PRINT,
        TokenType::TRUE_KW,
        TokenType::FALSE_KW
    ));
}

// ── TC-LEX-010: IDENTIFIER ────────────────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_010_Identifier) {
    auto tokens = tokenize("myVar");
    ASSERT_GE(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].getTokenType(), TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].getLexme(), "myVar");
}

// ── TC-LEX-011: 키워드 접두사를 가진 식별자 ──────────────────────────────────
// "ifx", "var2", "printme" 는 키워드가 아닌 IDENTIFIER 여야 한다
TEST_F(LexerTestFixture, TC_LEX_011_IdentifierWithKeywordPrefix) {
    auto result = types("ifx var2 printme");
    EXPECT_THAT(result, Each(TokenType::IDENTIFIER));
}

// ── TC-LEX-012: 단행 주석 제거 ───────────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_012_LineCommentIgnored) {
    auto result = types("+ // this is a comment\n-");
    EXPECT_THAT(result, ElementsAre(TokenType::PLUS, TokenType::MINUS));
}

// ── TC-LEX-013: 주석만 있는 경우 ─────────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_013_OnlyComment) {
    auto tokens = tokenize("// entire line is a comment");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].getTokenType(), TokenType::EOF_TOKEN);
}

// ── TC-LEX-014: 줄번호 추적 ──────────────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_014_LineTracking) {
    auto tokens = tokenize("var\nx");
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].getLine(), 1);  // var → 1번째 줄
    EXPECT_EQ(tokens[1].getLine(), 2);  // x   → 2번째 줄
}

// ── TC-LEX-015: 닫히지 않은 문자열 → LexError ────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_015_UnterminatedString) {
    EXPECT_THROW(tokenize("\"hello"), LexError);
}

// ── TC-LEX-016: 알 수 없는 문자 → LexError ───────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_016_UnknownCharacter) {
    EXPECT_THROW(tokenize("@"), LexError);
}

// ── TC-LEX-017: 복합 표현식 ──────────────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_017_CompoundExpression) {
    auto result = types("var x = 10 + 3.5;");
    EXPECT_THAT(result, ElementsAre(
        TokenType::VAR,
        TokenType::IDENTIFIER,
        TokenType::EQUAL,
        TokenType::NUMBER,
        TokenType::PLUS,
        TokenType::NUMBER,
        TokenType::SEMICOLON
    ));
}

// ── TC-LEX-018: true/false 리터럴 값 ─────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_018_BoolLiteralValues) {
    auto tokens = tokenize("true false");
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].getTokenType(), TokenType::TRUE_KW);
    EXPECT_EQ(std::get<bool>(tokens[0].getLiteral()), true);
    EXPECT_EQ(tokens[1].getTokenType(), TokenType::FALSE_KW);
    EXPECT_EQ(std::get<bool>(tokens[1].getLiteral()), false);
}

// ── TC-LEX-019: 전형적인 프로그램 스니펫 ──────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_019_TypicalProgramSnippet) {
    const std::string src = R"(
        var i = 0;
        for (i = 0; i < 10; i = i + 1) {
            print i;
        }
    )";
    auto tokens = tokenize(src);
    // 마지막이 EOF_TOKEN 인지만 확인 (전체 파싱은 Parser 테스트에서 수행)
    EXPECT_EQ(tokens.back().getTokenType(), TokenType::EOF_TOKEN);
    // 오류 없이 토큰화 완료 여부 확인: EOF 이전에 토큰이 존재해야 함
    EXPECT_GT(tokens.size(), 1u);
}

// ── TC-LEX-020: EOF 토큰 lexeme 확인 ─────────────────────────────────────────
TEST_F(LexerTestFixture, TC_LEX_020_EofTokenLexeme) {
    auto tokens = tokenize("1");
    const auto& eof = tokens.back();
    EXPECT_EQ(eof.getTokenType(), TokenType::EOF_TOKEN);
    EXPECT_EQ(eof.getLexme(), "");
}
