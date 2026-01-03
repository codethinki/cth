#include "test.hpp"

#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/this_coro.hpp"

namespace cth::co::this_coro {

TAG_TEST(executor_tag, returns_correct_executor) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<executor> { co_return co_await this_coro::executor; };

    auto const actual = sync_wait(exec, task());
    EXPECT_EQ(actual, exec);
}

}