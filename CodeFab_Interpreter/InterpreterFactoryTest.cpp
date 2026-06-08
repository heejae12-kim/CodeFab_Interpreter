#include "gmock/gmock.h"
#include "InterpreterFactory/InterpreterFactory.h"
#include <stdexcept>

TEST(InterpreterFactoryTest, InvalidRunModeThrows) {
    EXPECT_THROW(
        InterpreterFactory::create(static_cast<RunMode>(999)),
        std::invalid_argument
    );
}

TEST(InterpreterFactoryTest, FileRunModeCreatesRunner) {
    EXPECT_NO_THROW(
        InterpreterFactory::create(RunMode::FILE_RUN, "dummy.txt")
    );
}
