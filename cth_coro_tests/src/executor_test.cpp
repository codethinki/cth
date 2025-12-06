#include "test.hpp"

#include <cth/coro/executor.hpp>
#include <cth/coro/scheduler.hpp>
#include <cth/coro/executor_task.hpp>




namespace cth::co {

// ========== Construction Tests ==========

CORO_TEST(executor, construct_with_scheduler) {
    scheduler sched;
    executor exec(sched);
    SUCCEED();
}

CORO_TEST(executor, construct_from_reference) {
    scheduler sched;
    executor exec(sched);
    SUCCEED();
}

// ========== Copy/Move Semantics Tests ==========

CORO_TEST(executor, copy_constructible) {
    scheduler sched;
    executor exec1(sched);
    executor exec2(exec1);
    SUCCEED();
}

CORO_TEST(executor, copy_assignable) {
    scheduler sched1;
    scheduler sched2;
    executor exec1(sched1);
    executor exec2(sched2);

    exec2 = exec1;
    SUCCEED();
}

CORO_TEST(executor, move_constructible) {
    scheduler sched;
    executor exec1(sched);
    executor exec2(std::move(exec1));
    SUCCEED();
}

CORO_TEST(executor, move_assignable) {
    scheduler sched1;
    scheduler sched2;
    executor exec1(sched1);
    executor exec2(sched2);

    exec2 = std::move(exec1);
    SUCCEED();
}

// ========== Schedule Method Tests ==========

CORO_TEST(executor, schedule_returns_awaiter) {
    scheduler sched;
    executor exec(sched);

    auto awaiter = exec.schedule();

    // Verify it's an awaiter (has await_ready)
    SUCCEED();
}

CORO_TEST(executor, schedule_awaiter_compiles) {
    scheduler sched;
    executor exec(sched);

    [[maybe_unused]] auto awaiter = exec.schedule();
    SUCCEED();
}

// ========== Spawn Method Tests ==========

CORO_TEST(executor, spawn_method_compiles_with_void_task) {
    scheduler sched;
    executor exec(sched);

    auto task = []() -> executor_void_task { co_return; };

    [[maybe_unused]] auto spawned = exec.spawn(task());
    SUCCEED();
}

CORO_TEST(executor, spawn_method_compiles_with_value_task) {
    scheduler sched;
    executor exec(sched);
    static_assert(std::is_base_of_v<executor_promise_base, executor_task<int>::promise_type>, "fuck");
    auto task = []() -> executor_task<int> { co_return 42; };

    [[maybe_unused]] auto spawned = exec.spawn(task());
    SUCCEED();
}

// ========== Steal Method Tests ==========

CORO_TEST(executor, steal_method_compiles_with_foreign_awaitable) {
    scheduler sched;
    executor exec(sched);

    auto foreign_task = []() -> executor_void_task { co_return; };

    [[maybe_unused]] auto stolen = exec.steal(foreign_task());
    SUCCEED();
}

CORO_TEST(executor, steal_method_compiles_with_executor_task) {
    scheduler sched;
    executor exec(sched);

    auto task = []() -> executor_void_task { co_return; };

    [[maybe_unused]] auto stolen = exec.steal(task());
    SUCCEED();
}

CORO_TEST(executor, steal_forwards_executor_awaitable) {
    scheduler sched;
    executor exec(sched);

    auto task = []() -> executor_void_task { co_return; };
    auto original = task();

    // Steal should forward executor_awaitable unchanged
    [[maybe_unused]] auto stolen = exec.steal(std::move(original));
    SUCCEED();
}

// ========== Type Traits Tests ==========

CORO_TEST(executor, is_copy_constructible) { EXPECT_TRUE(std::is_copy_constructible_v<executor>); }

CORO_TEST(executor, is_copy_assignable) { EXPECT_TRUE(std::is_copy_assignable_v<executor>); }

CORO_TEST(executor, is_move_constructible) { EXPECT_TRUE(std::is_move_constructible_v<executor>); }

CORO_TEST(executor, is_move_assignable) { EXPECT_TRUE(std::is_move_assignable_v<executor>); }

// ========== Multiple Executor Tests ==========

CORO_TEST(executor, multiple_executors_same_scheduler) {
    scheduler sched;
    executor exec1(sched);
    executor exec2(sched);
    executor exec3(sched);

    SUCCEED();
}

CORO_TEST(executor, multiple_executors_different_schedulers) {
    scheduler sched1;
    scheduler sched2;
    executor exec1(sched1);
    executor exec2(sched2);

    SUCCEED();
}

// ========== Executor Lifetime Tests ==========

CORO_TEST(executor, executor_outlives_scheduler_reference) {
    scheduler sched;
    {
        executor exec(sched);
        // exec goes out of scope first
    }
    // sched still alive
    SUCCEED();
}

CORO_TEST(executor, copy_executor_shares_scheduler) {
    scheduler sched;
    executor exec1(sched);
    executor exec2 = exec1;

    // Both should reference the same scheduler
    SUCCEED();
}

} // namespace cth::co
