//THIS FILE IS 100% ai generated bc idgaf

#include "test.hpp"
#include "util/os_test_utilty.hpp"

#include <cth/coro/fence.hpp>
#include <cth/coro/scheduler.hpp>

#include <atomic>
#include <latch>
#include <barrier>
#include <thread>




namespace cth::co {

CORO_ITEST(scheduler, await_single_handle) {
    scheduler sched{autostart, 1};
    fence done;

    auto handle = os::create_event();

    sched.await(
        handle,
        [&] { done.signal(); }
    );

    os::signal_event(handle);
    done.wait();

    os::close_handle(handle);
}

CORO_ITEST(scheduler, await_multiple_handles) {
    scheduler sched{autostart, 2};
    constexpr int handleCount = 3;

    std::atomic<int> counter{0};
    std::latch allDone{handleCount};

    std::vector<scheduler::native_handle> handles;
    for(int i = 0; i < handleCount; ++i)
        handles.push_back(os::create_event());

    for(auto handle : handles) {
        sched.await(
            handle,
            [&] {
                counter.fetch_add(1, std::memory_order_relaxed);
                allDone.count_down();
            }
        );
    }

    for(auto handle : handles)
        os::signal_event(handle);

    allDone.wait();
    EXPECT_EQ(counter.load(), handleCount);

    for(auto handle : handles)
        os::close_handle(handle);
}

CORO_ITEST(scheduler, await_callback_runs_on_scheduler) {
    scheduler sched{autostart, 2};
    fence done;

    auto handle = os::create_event();

    sched.await(
        handle,
        [&] {
            EXPECT_TRUE(sched.owns_thread());
            done.signal();
        }
    );

    os::signal_event(handle);
    done.wait();

    os::close_handle(handle);
}

CORO_ITEST(scheduler, await_before_start) {
    scheduler sched{1};
    fence cbExecuted;

    auto handle = os::create_event();

    sched.await(
        handle,
        [&] { cbExecuted.signal(); }
    );

    os::signal_event(handle);
    EXPECT_FALSE(cbExecuted.signaled());

    sched.start();
    cbExecuted.wait();

    os::close_handle(handle);
}

CORO_ITEST(scheduler, await_delayed_signal) {
    scheduler sched{autostart, 1};
    fence cbExecuted;
    fence awaitRegistered;

    auto handle = os::create_event();

    sched.post([&] { awaitRegistered.signal(); });

    sched.await(
        handle,
        [&] { cbExecuted.signal(); }
    );

    awaitRegistered.wait();
    EXPECT_FALSE(cbExecuted.signaled());

    os::signal_event(handle);
    cbExecuted.wait();

    os::close_handle(handle);
}

CORO_ITEST(scheduler, await_with_post) {
    scheduler sched{autostart, 2};
    std::atomic<int> sequence{0};
    std::latch allDone{3};

    auto handle = os::create_event();

    sched.post(
        [&] {
            sequence.fetch_add(1, std::memory_order_relaxed);
            allDone.count_down();
        }
    );

    sched.await(
        handle,
        [&] {
            sequence.fetch_add(10, std::memory_order_relaxed);
            allDone.count_down();
        }
    );

    sched.post(
        [&] {
            sequence.fetch_add(100, std::memory_order_relaxed);
            allDone.count_down();
        }
    );

    os::signal_event(handle);
    allDone.wait();

    EXPECT_EQ(sequence.load(), 111);

    os::close_handle(handle);
}

CORO_ITEST(scheduler, await_concurrent) {
    constexpr size_t workers = 4;
    scheduler sched{autostart, workers};

    constexpr int handleCount = 8;
    std::atomic<int> counter{0};
    std::latch allDone{handleCount};

    std::vector<scheduler::native_handle> handles;
    for(int i = 0; i < handleCount; ++i)
        handles.push_back(os::create_event());

    for(auto handle : handles) {
        sched.await(
            handle,
            [&] {
                counter.fetch_add(1, std::memory_order_relaxed);
                allDone.count_down();
            }
        );
    }

    std::vector<std::jthread> signalers;
    for(auto handle : handles)
        signalers.emplace_back([handle] { os::signal_event(handle); });

    signalers.clear();
    allDone.wait();

    EXPECT_EQ(counter.load(), handleCount);

    for(auto handle : handles)
        os::close_handle(handle);
}

CORO_ITEST(scheduler, await_chained) {
    scheduler sched{autostart, 1};
    fence done;
    std::atomic<int> order{0};

    auto handle1 = os::create_event();
    auto handle2 = os::create_event();

    sched.await(
        handle1,
        [&] {
            EXPECT_EQ(order.fetch_add(1, std::memory_order_relaxed), 0);
            os::signal_event(handle2);
        }
    );

    sched.await(
        handle2,
        [&] {
            EXPECT_EQ(order.fetch_add(1, std::memory_order_relaxed), 1);
            done.signal();
        }
    );

    os::signal_event(handle1);
    done.wait();

    EXPECT_EQ(order.load(), 2);

    os::close_handle(handle1);
    os::close_handle(handle2);
}

} // namespace cth::co