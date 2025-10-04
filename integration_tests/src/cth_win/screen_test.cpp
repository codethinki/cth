#include "test.hpp"
#define WIN_SCREEN_TEST(suite, test_name) WIN_EX_TEST(_screen, suite, test_name)

#include "cth/win/screen.hpp"

#include "cth/win/string.hpp"
#include "cth/win/win_include.hpp"

namespace cth::win::screen {

WIN_SCREEN_TEST(create_window, unicode) {
    std::string const name{"( ͡° ͜ʖ ͡°)🦣"};

    auto [classOpt, handle] = create_window(name, {}, false);

    ASSERT_TRUE(handle != nullptr);
    ASSERT_TRUE(classOpt.has_value() && classOpt->id == to_wstring(name));
}

WIN_SCREEN_TEST(create_window, ascii) {
    std::string const name{"asdf"};

    auto [classOpt, handle] = create_window(name, {}, false);

    ASSERT_TRUE(handle != nullptr);
    ASSERT_TRUE(classOpt.has_value() && classOpt->id == to_wstring(name));
}

WIN_SCREEN_TEST(desktop_rect, main) {
    auto const rect = desktop_rect();
    ASSERT_TRUE(rect.width > 0 && rect.height > 0);
}

WIN_SCREEN_TEST(monitors, main) {
    auto const monitors = enum_monitors();

    ASSERT_TRUE(monitors.size() >= GetSystemMetrics(SM_CMONITORS));
}

}
