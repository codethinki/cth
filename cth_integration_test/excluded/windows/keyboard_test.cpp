
#include <gtest/gtest.h>

#include <print>

#include "cth/windows/keyboard.hpp"

namespace cth::keybd {
inline void callback(uint32_t key, uint32_t data) { std::println("callback: key \'{0}\', action: {1}", static_cast<char>(key), data); }
inline void rawCallback(KBDLLHOOKSTRUCT const& key, WPARAM action) { std::cout << "rawCallback" << &key << &action << std::endl; }

//BUG fix this

TEST(EventQueue, constructor_destructor) {
  /*  [[maybe_unused]] win::keybd::EventQueue queue{};
    [[maybe_unused]] win::keybd::RawEventQueue rawQueue{};
    */
}
TEST(CallbackEventQueue, constructor_destructor) {
    /*
    [[maybe_unused]] win::keybd::CallbackEventQueue callbackEventQueue{callback};
    [[maybe_unused]] win::keybd::RawCallbackEventQueue rawCallbackEventQueue{rawCallback};*/
}

} // namespace cth
