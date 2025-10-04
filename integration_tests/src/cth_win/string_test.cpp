#include "test.hpp"

#include "cth/win/string.hpp"

namespace cth::win {
std::string const utf8{"( ͡° ͜ʖ ͡°)🦣"};
std::wstring const utf16{L"( ͡° ͜ʖ ͡°)🦣"};

WIN_TEST(to_wstring, from_utf8_string) {
    auto const actual = to_wstring(utf8);
    ASSERT_EQ(actual, utf16);
}
WIN_TEST(to_string, from_utf16_string) {
    auto const actual = to_string(utf16);
    ASSERT_EQ(actual, utf8);
}
}
