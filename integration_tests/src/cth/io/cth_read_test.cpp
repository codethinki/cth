#include "test.hpp"

#define FILE_TEST(suite, test_name) IO_EX_TEST(_file, suite, test_name)

#include "cth/io/file.hpp"


namespace cth::io::file {

FILE_TEST(chop, main) {
    std::println("working dir: {}", std::filesystem::current_path().string());
    constexpr std::string_view path = "res/io/file/text.txt";

    auto const result = chop(path);
    auto const result2 = chop(path, ' ');
}
}
