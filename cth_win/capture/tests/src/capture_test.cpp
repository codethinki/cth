#include "test.hpp"

#include "cth/win/capture.hpp"

#include "cth/win/ui/screen.hpp"

#include <chrono>


namespace cth::win::capture {

WIN_CAPTURE_TEST(supported, main) { EXPECT_NO_THROW(supported()); }


WIN_CAPTURE_TEST(window_capture, frame) {
    if(!supported()) GTEST_SKIP() << "window capture not supported";

    auto const window = ui::create_window("cth window capture test", {100, 100, 300, 200});

    window_capture capture{window.handle.get()};

    auto const view = capture.next_frame(std::chrono::seconds{5});

    EXPECT_FALSE(view.empty());
    EXPECT_EQ(view.size(), capture.bytes());
    EXPECT_GT(capture.width(), 0);
    EXPECT_GT(capture.height(), 0);
}

}
