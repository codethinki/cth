#include "cth/io/log.hpp"

#include "cth/windows.hpp"

#include <chrono>
#include <gtest/gtest.h>



namespace cth::win::proc {
    TEST(elevated, main) { EXPECT_FALSE(cth::win::proc::elevated()); }

    TEST(enumerate, main) {
        auto const& processIds = cth::win::proc::enumerate();
        EXPECT_FALSE(processIds.empty());
    }

    TEST(name, main) {
        auto const& processIds = cth::win::proc::enumerate();
        size_t names = 0;
        for(auto const id : processIds) {
            auto const str = cth::win::proc::name(id, false);
            if(str.has_value()) ++names;
        }

        EXPECT_TRUE(names > 0);
    }

    TEST(instances, main) {

        auto const instances = cth::win::proc::instances(L"explorer.exe");
        EXPECT_TRUE(instances.has_value());
        EXPECT_TRUE(instances.value() > 0);
    }
    TEST(active, main) {
        auto const active = cth::win::proc::active(L"explorer.exe");
        EXPECT_TRUE(active.has_value());
    }
}

namespace cth::win::cmd {
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
}