#include "test.hpp"

#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/this_coro.hpp"

#include <chrono>

namespace cth::co::this_coro {

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