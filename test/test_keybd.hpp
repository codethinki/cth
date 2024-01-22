#pragma once
#include <gtest/gtest.h>

#include "../cth/cth_keybd.hpp"

namespace cth {
    void callback(int x, int y) {
        std::cout << "callback" << std::endl;
    }
    void rawCallback(const KBDLLHOOKSTRUCT& x, WPARAM y) {
        std::cout << "rawCallback" << std::endl;
    }

    TEST(headerKeybd, classKeyBd) {
        EXPECT_EQ(1, 1);
        keybd::EventQueue queue{};
        keybd::RawEventQueue rawQueue{};
        keybd::CallbackEventQueue callbackEventQueue{callback};
        keybd::RawCallbackEventQueue rawCallbackEventQueue{rawCallback};

    }

}