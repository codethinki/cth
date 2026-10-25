#include "test.hpp"

#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/this_coro.hpp"

namespace cth::co::this_coro {
// AI

TAG_TEST(context_tag, returns_correct_context) {
    co::scheduler sched{autostart, 1};
    co::executor exec{sched};

    using r_t = co::context const&;

    auto task = []() -> executor_task<r_t> { co_return co_await this_coro::context; };

    static_assert(std::same_as<decltype(exec.sync_wait(task())), r_t>);


    auto& actual = exec.sync_wait(task());
    EXPECT_EQ(&actual, &sched);
}

}
