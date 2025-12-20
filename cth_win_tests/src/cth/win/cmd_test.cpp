#include "cth/win/cmd.hpp"

#include "test.hpp"

namespace cth::win::cmd {
cxpr std::string_view PATH = "res\\";


WIN_TEST(hidden_dir, basic) {
    size_t result;

    EXPECT_NO_THROW((result = hidden_dir(PATH, "dir")));


    EXPECT_EQ(result, 0);
}
WIN_TEST(hidden_dir, format) {
    auto const result = hidden_dir(PATH, "di{}", "r");

    EXPECT_EQ(result, 0);
}

WIN_TEST(hidden_dir, invalidCommand) {
    auto const result = hidden_dir(PATH, "non_existing_command");
    EXPECT_NE(result, 0);
}

} // namespace cth::win::cmd
