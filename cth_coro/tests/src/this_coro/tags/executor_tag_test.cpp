#include "test.hpp"

#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/this_coro.hpp"

namespace cth::co::this_coro {
// AI

TAG_TEST(executor_tag, returns_correct_executor) {
    co::scheduler sched{autostart, 1};
    co::executor exec{sched};

    auto task = []() -> executor_task<co::executor> { co_return co_await this_coro::executor; };

    auto const actual = sync_wait(exec, task());
    EXPECT_EQ(actual, exec);
}

}
