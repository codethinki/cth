#include "test.hpp"

#define WIN_IO_TEST(suite, test_name) WIN_EX_TEST(_io, suite, test_name)

#include "cth/win/io.hpp"

#include "cth/io/log.hpp"
#include "cth/win/string.hpp"

#include <filesystem>
#include <fstream>

namespace cth::win::io {
std::vector<char> load_file_bytes(std::string_view path) {
    std::ifstream file{to_wstring(path), std::ios::binary};


    CTH_STABLE_ABORT(!file.is_open(), "failed to open test file: {}", path) {}

    // determine file size
    file.seekg(0, std::ios::end);
    auto const size = file.tellg();
    file.seekg(0, std::ios::beg);

    CTH_STABLE_ABORT(size <= 0, "invalid test file size: {}", path) {}
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);

    return buffer;
}


WIN_IO_TEST(read_unbuffered, main) {
    std::string const path{"res/cth_win/io/unicode🐸_image.jpg"};
    std::vector<char> buffer{};
    read_unbuffered(path, buffer); //TEMP fix the cmd test 

    ASSERT_EQ(buffer, load_file_bytes(path));
}

WIN_IO_TEST(read_clipboard, main) {
    auto result = read_clipboard();
    ASSERT_TRUE(result.has_value());
}
}
