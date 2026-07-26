#include "test.hpp"

#include "cth/win/coro/timer.hpp"


namespace cth::win::co {

CO_TEST(timer, auto_reset) {
    timer t{};
    t.set(std::chrono::steady_clock::now());

    EXPECT_EQ(t.wait(100), wait_result::WAITED);


    EXPECT_EQ(t.wait(0), wait_result::TIMEOUT);
}

CO_TEST(timer, manual_reset) {
    timer t{false};
    t.set(std::chrono::steady_clock::now());

    EXPECT_EQ(t.wait(100), wait_result::WAITED);

    EXPECT_EQ(t.wait(0), wait_result::WAITED);

    t.reset();

    EXPECT_EQ(t.wait(0), wait_result::TIMEOUT);
}

CO_TEST(timer, hash) {
    timer t{};
    auto val = std::hash<timer>{}(t);
}
}
