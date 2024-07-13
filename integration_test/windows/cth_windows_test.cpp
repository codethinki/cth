#include "../../cth/cth_windows.hpp"
#include "../../cth/io/cth_log.hpp"

#include <chrono>
#include <gtest/gtest.h>



namespace cth::win {


namespace proc {
    TEST(elevated, main) { EXPECT_FALSE(cth::win::proc::elevated()); }
}

namespace cmd {
    TEST(hidden, main) {
        [[maybe_unused]] int x = 0;
        x = win::cmd::hidden("hello");
        x = win::cmd::hidden("hello {}", "asdf");
    }
    TEST(hidden_dir, main) {
        [[maybe_unused]] int x = 0;

        x = win::cmd::hidden_dir(std::filesystem::current_path().string(), "hello");
        x = win::cmd::hidden_dir(std::filesystem::current_path().string(), "hello {}", "hello");
    }
} //namespace cmd

}
