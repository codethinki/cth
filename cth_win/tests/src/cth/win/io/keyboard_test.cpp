#include "cth/win/io/keyboard.hpp"

#include "test.hpp"

#include "cth/io/log.hpp"

#include <array>

#define KEYBD_TEST(suite, name) WIN_EX_TEST(keybd, suite, name)

namespace cth::win::keybd {

/*
KEYBD_TEST(send, asdf) {
    std::this_thread::sleep_for(std::chrono::milliseconds{2000});

    constexpr std::array<io::key_state, 6> keys{
        {
            {io::Key::A, false, true},
            {io::Key::A, false, false},
            {io::Key::S, false, true},
            {io::Key::S, false, false},
            {io::Key::S, false, true},
            {io::Key::S, false, false},
        }
    };
    send(keys);
}

KEYBD_TEST(event_queue, test_recording) {
    event_queue q{};

    std::this_thread::sleep_for(std::chrono::seconds{4});

    std::println("{}", q.pop_queue());
}
*/

KEYBD_TEST(event_queue, basic) {
    event_queue queue{};
    CTH_CRITICAL(queue.empty(), "event queue should be empty after construction");

    queue.clear();
    auto events = queue.pop_queue();
    CTH_CRITICAL(events.empty(), "pop_queue should return an empty vector for empty queue");
}
}
