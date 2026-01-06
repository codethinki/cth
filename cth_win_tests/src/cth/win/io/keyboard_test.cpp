#include "cth/win/io/keyboard.hpp"

#include "test.hpp"

#define KEYBD_TEST(suite, name) WIN_EX_TEST(keybd, suite, name)

namespace cth::win {
KEYBD_TEST(send, main) {
 /*   std::this_thread::sleep_for(std::chrono::seconds{2});

    std::array<io::Key, 5> keys{
        io::Key::H,
        io::Key::E,
        io::Key::L,
        io::Key::L,
        io::Key::O
    };

    std::vector<io::key_state> updates{};
    updates.reserve(keys.size() * 2);

    for(auto& key : keys) {
        updates.emplace_back(io::ex_key{key}, true);
        updates.emplace_back(io::ex_key{key}, false);
    }
    keybd::send(updates);*/
}
}
