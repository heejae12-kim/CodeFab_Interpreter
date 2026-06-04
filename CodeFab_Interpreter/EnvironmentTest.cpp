#include "gmock/gmock.h"
#include "Environment.h"
#include "TestHelpers.h"
#include <memory>

using namespace tst;

// 공통 셋업을 fixture 로 추출:
//   - env   : 단일(독립) 스코프
//   - global : 상위 스코프, local : global 을 enclosing 으로 갖는 지역 스코프
class EnvironmentTest : public ::testing::Test {
protected:
    Environment env;
    std::shared_ptr<Environment> global = std::make_shared<Environment>();
    Environment local{ global };
};

TEST_F(EnvironmentTest, DefineThenGetReturnsValue) {
    env.define("a", 3.0);

    EXPECT_EQ(asNum(env.get(nameTok("a"))), 3.0);
}

TEST_F(EnvironmentTest, GetUndefinedVariableThrowsRuntimeError) {
    EXPECT_THROW(env.get(nameTok("x")), RuntimeError);
}

TEST_F(EnvironmentTest, GetUndefinedReportsExactMessageAndLine) {
    try {
        env.get(nameTok("x", 7));
        FAIL() << "RuntimeError 가 발생해야 합니다";
    }
    catch (const RuntimeError& e) {
        EXPECT_EQ(e.token.getLine(), 7);
        EXPECT_STREQ(e.what(), "Undefined variable 'x'."); // 전체 메시지 정확히 검증
    }
}

TEST_F(EnvironmentTest, AssignUpdatesExistingVariable) {
    env.define("a", 1.0);

    env.assign(nameTok("a"), 2.0);

    EXPECT_EQ(asNum(env.get(nameTok("a"))), 2.0);
}

TEST_F(EnvironmentTest, AssignUndefinedReportsExactMessage) {
    try {
        env.assign(nameTok("x"), 1.0);
        FAIL() << "RuntimeError 가 발생해야 합니다";
    }
    catch (const RuntimeError& e) {
        EXPECT_STREQ(e.what(), "Undefined variable 'x'.");
    }
}

TEST_F(EnvironmentTest, GetWalksUpToEnclosingScope) {
    global->define("ga", 3.0);

    EXPECT_EQ(asNum(local.get(nameTok("ga"))), 3.0);
}

TEST_F(EnvironmentTest, LocalDefinitionShadowsEnclosing) {
    global->define("a", 1.0);
    local.define("a", 7.0);

    EXPECT_EQ(asNum(local.get(nameTok("a"))), 7.0);
    EXPECT_EQ(asNum(global->get(nameTok("a"))), 1.0); // 상위 스코프는 유지
}

TEST_F(EnvironmentTest, AssignWalksUpToEnclosingScope) {
    global->define("a", 1.0);

    local.assign(nameTok("a"), 9.0);

    EXPECT_EQ(asNum(global->get(nameTok("a"))), 9.0);
}
