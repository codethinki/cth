#include <gtest/gtest.h>

#include "cth/pointer/not_null.hpp"

namespace cth {
TEST(not_null, nullptr_crash) {
    std::unique_ptr<int> constexpr nullPtr{};

#ifdef CTH_DEBUG_MODE
    ASSERT_DEATH([[maybe_unused]] not_null ptr{nullPtr.get()}, ".*");
#endif

}
}
