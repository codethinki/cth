#include "test.hpp"

#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/this_coro.hpp"

namespace cth::co {

THIS_CORO_TEST(promise_base, tag_injection) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<executor> { co_return co_await this_coro::executor; };

    auto const actual = sync_wait(exec, task());
    EXPECT_EQ(actual, exec);
}

THIS_CORO_TEST(promise_base, nested_task_propagation) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto child = []() -> executor_task<executor> { co_return co_await this_coro::executor; };

    auto root = [](auto c) -> executor_task<executor> { co_return co_await c(); };

    auto const actual = sync_wait(exec, root(child));
    EXPECT_EQ(actual, exec);
}

THIS_CORO_TEST(promise_base, deep_recursion_propagation) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    struct Recursive {
        auto operator()(int depth) -> executor_task<executor> {
            if(depth == 0) {
                co_return co_await this_coro::executor;
            }
            co_return co_await (*this)(depth - 1);
        }
    };

    auto const actual = sync_wait(exec, Recursive{}(10));
    EXPECT_EQ(actual, exec);
}

}
