#include "cth/coro/this_coro.hpp"

#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"

#include "test.hpp"

#define THIS_CORO_TEST(suite, name) CORO_EX_TEST(_this_coro, suite, name)


namespace cth::co::this_coro {
co::scheduler sched{autostart};
co::executor exec{sched};

THIS_CORO_TEST(scheduler_tag, main) {
    auto const task = []() -> cth::co::executor_task<co::scheduler&> {
        co_return co::this_coro::scheduler;
    };

    auto spawned = exec.spawn(task());


}

THIS_CORO_TEST(executor_tag, main) {}
}
