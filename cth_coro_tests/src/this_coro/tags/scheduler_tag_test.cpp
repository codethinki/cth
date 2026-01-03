#include "cth/coro/this_coro.hpp"

#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"

#include "test.hpp"

namespace cth::co::this_coro {
co::scheduler sched{autostart};
co::executor exec{sched};

THIS_CORO_TEST(scheduler_tag, main) {
    auto const task = []() -> cth::co::executor_task<co::scheduler&> {
        co_return co_await co::this_coro::scheduler;
    };

    auto spawned = exec.spawn(task());
}

}
