#include "cth/win/windows.hpp"

#include "test.hpp"

namespace cth::win::cmd {
    WIN_TEST(hidden_dir, main) {
        auto const result = hidden_dir("res/", "{}", "list");

        ASSERT_EQ(result, 0);
    }
}