#include "test.hpp"

#include "cth/os/timer.hpp"

#include <chrono>


namespace cth::os {
using namespace std::chrono_literals;

OS_TEST(timer, auto_reset) {
    timer t{};
    t.set(timer::clock_t::now());

    EXPECT_EQ(t.wait(100ms), wait_result::WAITED);

    EXPECT_EQ(t.wait(0ms), wait_result::TIMEOUT);
}

OS_TEST(timer, manual_reset) {
    timer t{false};
    t.set(timer::clock_t::now());

    EXPECT_EQ(t.wait(100ms), wait_result::WAITED);

    EXPECT_EQ(t.wait(0ms), wait_result::WAITED);

    t.reset();

    EXPECT_EQ(t.wait(0ms), wait_result::TIMEOUT);
}

// the whole reason this is not asio's steady_timer: sub millisecond resolution
OS_TEST(timer, sub_ms_resolution) {
    timer t{};

    auto const start = timer::clock_t::now();
    t.set(start + 50us);

    ASSERT_EQ(t.wait(100ms), wait_result::WAITED);

    EXPECT_LT(timer::clock_t::now() - start, 1ms);
}

OS_TEST(timer, hash) {
    timer t{};
    [[maybe_unused]] auto const val = std::hash<timer>{}(t);
}

}
