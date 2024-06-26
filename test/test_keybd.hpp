#pragma once
#include <gtest/gtest.h>

#include <print>

#include "../cth/cth_keybd.hpp"

namespace cth::keybd {
inline void callback(const uint32_t key, uint32_t data) { std::println("callback: key \'{0}\', action: {1}", static_cast<char>(key), data); }
inline void rawCallback(const KBDLLHOOKSTRUCT& key, const WPARAM action) { std::cout << "rawCallback" << &key << &action << std::endl; }

TEST(class_EventQueue, constructor_destructor) {
  /*  [[maybe_unused]] win::keybd::EventQueue queue{};
    [[maybe_unused]] win::keybd::RawEventQueue rawQueue{};
    */
}
TEST(class_CallbackEventQueue, constructor_destructor) {
    /*
    [[maybe_unused]] win::keybd::CallbackEventQueue callbackEventQueue{callback};
    [[maybe_unused]] win::keybd::RawCallbackEventQueue rawCallbackEventQueue{rawCallback};*/
}

} // namespace cth
