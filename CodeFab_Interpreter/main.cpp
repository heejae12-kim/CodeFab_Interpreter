#ifdef _DEBUG
#include "gmock/gmock.h"

int main() {
    testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}

#else

int main() {
    // Release: 사용자 입력 모드 (추후 구현 예정)
    return 0;
}

#endif
