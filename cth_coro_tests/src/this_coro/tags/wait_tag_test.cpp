#include "test.hpp"

#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/this_coro.hpp"
#include "cth/coro/tasks/executor_task.hpp"

#include <chrono>

namespace cth::co::this_coro {


TAG_TEST(wait_tag, sub_1ms) {
    co::scheduler sched{autostart, 1};

    co::executor exec{sched};

    auto task = []() -> executor_task<void> {
        co_await this_coro::wait(std::chrono::microseconds{50});
        co_return;
    };


    auto start = clock_t::now();
    sync_wait(exec, task());
    auto end = clock_t::now();
    EXPECT_LT(end, start + std::chrono::milliseconds{1});


    for(size_t i = 0; i < 10; i++) {
        auto start2 = clock_t::now();
        sync_wait(exec, task());
        auto end2 = clock_t::now();

        std::println("waited for: {}", std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2));
    }

}

TAG_TEST(wait_tag, immediate_wait) {
    co::scheduler sched{autostart, 1};
    co::executor exec{sched};

    auto now = std::chrono::steady_clock::now();
    auto task = [](auto timepoint) -> executor_task<void> { co_await wait_tag{timepoint}; };

    sync_wait(exec, task(now));
}

TAG_TEST(wait_tag, future_wait) {
    co::scheduler sched{autostart, 1};
    co::executor exec{sched};

    auto now = std::chrono::steady_clock::now();
    auto future = now + std::chrono::milliseconds{10};

    auto task = [](auto timepoint) -> executor_task<bool> {
        auto start = std::chrono::steady_clock::now();
        co_await wait_tag{timepoint};
        auto elapsed = std::chrono::steady_clock::now() - start;
        co_return elapsed >= std::chrono::milliseconds{5};
    };

    bool const waited = sync_wait(exec, task(future));
    EXPECT_TRUE(waited);
}

TAG_TEST(wait_tag, multiple_sequential_waits) {
    co::scheduler sched{autostart, 1};
    co::executor exec{sched};

    auto now = std::chrono::steady_clock::now();
    auto task = [](auto timepoint) -> executor_task<int> {
        co_await wait_tag{timepoint};
        co_await wait_tag{timepoint + std::chrono::milliseconds{5}};
        co_return 42;
    };

    int const result = sync_wait(exec, task(now));
    EXPECT_EQ(result, 42);
}

}
