#include "test.hpp"


#include "cth/win/io.hpp"
#include "cth/win/string.hpp"

#include <cth/io/log.hpp>

#include <filesystem>
#include <fstream>

namespace cth::win::io {
std::vector<std::byte> load_file_bytes(std::string_view path) {
    std::ifstream file{to_wstring(path), std::ios::binary};


    CTH_STABLE_ABORT(!file.is_open(), "failed to open test file: {}", path) {}

    // determine file size
    file.seekg(0, std::ios::end);
    auto const size = file.tellg();
    file.seekg(0, std::ios::beg);

    CTH_STABLE_ABORT(size <= 0, "invalid test file size: {}", path) {}
    std::vector<std::byte> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    return buffer;
}


WIN_IO_TEST(read_unbuffered_with_unicode_path, main) {
    std::string const path{"res/cth/win/io/unicode🐸_image.jpg"};
    std::vector<std::byte> buffer{};
    read_unbuffered(path, buffer); //TEMP fix the cmd test 

    EXPECT_EQ(buffer, load_file_bytes(path));
}

WIN_IO_TEST(read_clipboard, main) {
    auto result = read_clipboard();
    EXPECT_TRUE(result.has_value());
}
}
