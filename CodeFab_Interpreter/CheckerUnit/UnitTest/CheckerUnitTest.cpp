#include "gmock/gmock.h"
#include "../CheckerUnit.h"

using std::vector;

using namespace testing;

TEST(CheckerUnit, EmptyProgram) {
	CheckerUnit checker_unit;
	vector<StmtPtr> input = {};
	EXPECT_NO_THROW(checker_unit.DoChecker(input));
}
