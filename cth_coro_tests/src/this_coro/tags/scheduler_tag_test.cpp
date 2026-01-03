#include "test.hpp"

#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/this_coro.hpp"

namespace cth::co::this_coro {

TAG_TEST(scheduler_tag, returns_correct_scheduler) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<scheduler&> { co_return co_await this_coro::scheduler; };

    auto& actual = sync_wait(exec, task());
    EXPECT_EQ(&actual, &sched);
}

}