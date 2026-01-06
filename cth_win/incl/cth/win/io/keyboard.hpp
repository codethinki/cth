#pragma once
#include "cth/io/keybd/keys.hpp"

#include <span>

namespace cth::win {
class keybd {
public:
    static void send(io::key_state);
    static void send(std::span<io::key_state const> states);
};

}
