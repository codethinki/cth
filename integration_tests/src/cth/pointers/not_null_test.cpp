#include <gtest/gtest.h>

#include "cth/pointer/not_null.hpp"

namespace cth {
#ifdef _DEBUG
TEST(not_null, nullptr_crash) {
    std::unique_ptr<int> constexpr nullPtr{};

    ASSERT_DEATH(not_null{nullPtr.get()}, ".*");
}
#endif
}
