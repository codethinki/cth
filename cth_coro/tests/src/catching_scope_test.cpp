#include "test.hpp"

#include "cth/coro/catching_scope.hpp"
#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/utility/concepts.hpp"

#include <atomic>
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <thread>
#include <vector>


namespace cth::co {
namespace {
    static_assert(detach_scope<simple_counting_scope>);
    static_assert(detach_scope<catching_scope>);

    executor_task<void> throw_value(int value) {
        throw value;
        co_return;
    }

    /**
     * @return the int that `rethrow()` throws, `-1` -> another exception
     */
    [[nodiscard]] int rethrown_value(catching_scope const& scope) {
        try { scope.rethrow(); }
        catch(int value) { return value; }
        catch(...) { return -1; }
        return -1;
    }
}

CORO_TEST(catching_scope, a_throwing_detached_task_stores_its_exception) {
    scheduler sched{autostart, 2};
    catching_scope tasks{};

    auto task = []() -> executor_task<void> {
        throw std::runtime_error{"detached"};
        co_return;
    };
    executor{sched}.detach(tasks, task());
    tasks.wait();

    ASSERT_TRUE(tasks.failed());
    EXPECT_THROW(tasks.rethrow(), std::runtime_error);
}

CORO_TEST(catching_scope, a_task_without_an_exception_stores_nothing) {
    scheduler sched{autostart, 2};
    catching_scope tasks{};
    std::atomic<bool> ran{false};

    auto task = [](std::atomic<bool>& flag) -> executor_task<void> {
        flag = true;
        co_return;
    };
    executor{sched}.detach(tasks, task(ran));
    tasks.wait();

    EXPECT_TRUE(ran);
    EXPECT_FALSE(tasks.failed());
}

CORO_TEST(catching_scope, tasks_that_throw_at_once_keep_one_error) {
    constexpr int TASKS = 2000;
    scheduler sched{autostart, 8};
    catching_scope tasks{};

    executor const exec{sched};
    for(int i = 0; i < TASKS; ++i)
        exec.detach(tasks, throw_value(i));
    tasks.wait();

    ASSERT_TRUE(tasks.failed());
    auto const kept = rethrown_value(tasks);
    EXPECT_GE(kept, 0);
    EXPECT_LT(kept, TASKS);
    // the later stores dropped their errors: the kept error stays
    EXPECT_EQ(rethrown_value(tasks), kept);
}

CORO_TEST(catching_scope, stores_from_several_threads_keep_one_error) {
    constexpr int THREADS = 8;
    constexpr int STORES = 1000;
    catching_scope scope{};

    std::atomic<bool> go{false};
    std::vector<std::thread> threads{};
    for(int t = 0; t < THREADS; ++t)
        threads.emplace_back([&scope, &go, t] {
            while(!go) std::this_thread::yield();
            for(int i = 0; i < STORES; ++i)
                scope.store(std::make_exception_ptr(t * STORES + i));
        });
    go = true;
    for(auto& thread : threads)
        thread.join();

    ASSERT_TRUE(scope.failed());
    auto const kept = rethrown_value(scope);
    EXPECT_GE(kept, 0);
    EXPECT_LT(kept, THREADS * STORES);
}

CORO_TEST(catching_scope, store_outside_of_a_task_keeps_the_first_error) {
    catching_scope scope{};
    EXPECT_FALSE(scope.failed());

    scope.store(std::make_exception_ptr(1));
    scope.store(std::make_exception_ptr(2));

    ASSERT_TRUE(scope.failed());
    EXPECT_EQ(rethrown_value(scope), 1);
}

CORO_TEST(catching_scope, clear_forgets_the_error) {
    catching_scope scope{};
    scope.store(std::make_exception_ptr(1));

    scope.clear();
    EXPECT_FALSE(scope.failed());

    scope.store(std::make_exception_ptr(2));
    ASSERT_TRUE(scope.failed());
    EXPECT_EQ(rethrown_value(scope), 2);
}
}
