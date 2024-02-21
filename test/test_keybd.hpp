#pragma once
#include <gtest/gtest.h>
#include <print>

#include "../cth/cth_keybd.hpp"

namespace cth {
void callback(uint32_t key, uint32_t data) { std::println("callback: key \'{0}\', action: {1}", static_cast<char>(key), data); }
void rawCallback(const KBDLLHOOKSTRUCT& key, WPARAM action) { std::cout << "rawCallback" << std::endl; }

TEST(headerKeybd, classKeyBd) {
    try {
            win::keybd::EventQueue queue{};
            win::keybd::RawEventQueue rawQueue{};
            win::keybd::CallbackEventQueue callbackEventQueue{callback};
            win::keybd::RawCallbackEventQueue rawCallbackEventQueue{rawCallback};
    }
    catch(cth::except::default_exception<cth::except::ERR>& e) {
        FAIL() << "Exception thrown";
        EXPECT_EQ(0, 1);
    }
}

}
