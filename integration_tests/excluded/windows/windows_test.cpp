#include "test.hpp"

#include "cth/io/log.hpp"

#include "cth/windows.hpp"

#include <chrono>


#define PROC_TEST(suite, test_name) WIN_EX_TEST(_proc, suite, test_name)


namespace cth::win::proc {
PROC_TEST(elevated, main) { EXPECT_FALSE(cth::win::proc::elevated()); }

PROC_TEST(enumerate, main) {
    auto const& processIds = enumerate();
    EXPECT_FALSE(processIds.empty());
}

PROC_TEST(name, main) {
    auto const& processIds = enumerate();
    size_t names = 0;
    for(auto const id : processIds) {
        auto const str = name(id, false);
        if(str.has_value())
            ++names;
    }

    EXPECT_TRUE(names > 0);
}

PROC_TEST(instances, main) {
    auto const instances = proc::count(L"explorer.exe");
    EXPECT_TRUE(instances.has_value());
    EXPECT_TRUE(instances.value() > 0); // NOLINT(bugprone-unchecked-optional-access)
}
PROC_TEST(active, main) {
    auto const active = proc::active(L"explorer.exe");
    EXPECT_TRUE(active.has_value());
}
}

#define CMD_TEST(suite, test_name) CTH_EX_TEST(_cmd, suite, test_name)

namespace cth::win::cmd {
CMD_TEST(hidden, main) {
    [[maybe_unused]] int x = 0;
    x = hidden("hello");
    x = hidden("hello {}", "asdf");
}
CMD_TEST(hidden_dir, main) {
    [[maybe_unused]] int x = 0;

    x = hidden_dir(std::filesystem::current_path().string(), "hello");
    x = hidden_dir(std::filesystem::current_path().string(), "hello {}", "hello");
}
}
