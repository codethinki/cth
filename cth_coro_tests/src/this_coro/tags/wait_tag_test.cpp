#include "test.hpp"

#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/this_coro.hpp"

#include <chrono>

namespace cth::co::this_coro {
co::scheduler sched{autostart, 1};

co::executor exec{sched};

TAG_TEST(wait_tag, sub_1ms) {
    auto task = []() -> executor_task<void> {
        co_await wait_tag{clock_t::now() + std::chrono::microseconds{50}};
        co_return;
    };


    auto start = clock_t::now();
    sync_wait(exec, task());
    auto end = clock_t::now();
    EXPECT_LT(end, start + std::chrono::milliseconds{1});


    /* for(size_t i = 0; i < 10; i++) {
         auto start2 = clock_t::now();
         sync_wait(exec, task());
         auto end2 = clock_t::now();

         std::println("waited for: {}", std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2));
     }*/
}

TAG_TEST(wait, main) {
    auto task = []() -> executor_task<void> {
        co_await this_coro::wait(std::chrono::microseconds{50});
        co_return;
    };

    sync_wait(exec, task());
}

TAG_TEST(wait_until, main) {
    auto task = []() -> executor_task<void> {
        co_await this_coro::wait_until(clock_t::now() + std::chrono::microseconds{50});
        co_return;
    };

    sync_wait(exec, task());
}


// IMPLEMENT concurrent wait test
}
