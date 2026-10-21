#include "test.hpp"

#include "cth/coro/global_switch.hpp"

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>


namespace cth::co {
using namespace std::chrono_literals;

CORO_TEST(global_switch, first_acquire_and_last_release_switch) {
    int on = 0;
    int off = 0;
    global_switch sw{[&] { ++on; }, [&] { ++off; }};

    EXPECT_TRUE(sw.acquire());
    EXPECT_FALSE(sw.acquire());
    EXPECT_EQ(on, 1);
    EXPECT_EQ(sw.acquire_count(), 2);

    EXPECT_FALSE(sw.release());
    EXPECT_EQ(off, 0);
    EXPECT_TRUE(sw.release());
    EXPECT_EQ(off, 1);
    EXPECT_FALSE(sw.acquired());
}

CORO_TEST(global_switch, signaled_acquires_and_destruction_switches_off) {
    int off = 0;
    {
        global_switch const sw{signaled, [] {}, [&] { ++off; }};
        EXPECT_TRUE(sw.acquired());
    }
    EXPECT_EQ(off, 1);
}

CORO_TEST(global_switch, throwing_on_leaves_the_count_unchanged) {
    int on = 0;
    bool fail = true;
    global_switch sw{
        [&] {
            ++on;
            if(fail) throw std::runtime_error{"on failed"};
        },
        [] {}
    };

    EXPECT_THROW(sw.acquire(), std::runtime_error);
    EXPECT_FALSE(sw.acquired());

    fail = false;
    EXPECT_TRUE(sw.acquire());
    EXPECT_EQ(on, 2);
    sw.release();
}

CORO_TEST(global_switch, concurrent_acquire_returns_after_on_finished) {
    std::atomic<bool> entered = false;
    std::atomic<bool> finished = false;
    global_switch sw{
        [&] {
            entered = true;
            entered.notify_all();
            std::this_thread::sleep_for(20ms);
            finished = true;
        },
        [] {}
    };

    std::jthread first{[&] { sw.acquire(); }};
    entered.wait(false);

    sw.acquire();
    EXPECT_TRUE(finished);

    first.join();
    sw.release();
    sw.release();
}

CORO_TEST(global_switch, on_waits_for_a_running_off) {
    std::atomic<bool> inOff = false;
    bool onDuringOff = false;
    global_switch sw{
        [&] { onDuringOff = inOff; },
        [&] {
            inOff = true;
            inOff.notify_all();
            std::this_thread::sleep_for(20ms);
            inOff = false;
        }
    };
    sw.acquire();

    std::jthread last{[&] { sw.release(); }};
    inOff.wait(false);

    sw.acquire();
    EXPECT_FALSE(onDuringOff);

    last.join();
    sw.release();
}
}
