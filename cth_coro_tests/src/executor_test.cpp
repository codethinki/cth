#include "test.hpp"
#include "cth/coro/fence.hpp"
#include "cth/coro/tasks/sync_task.hpp"

#include <cth/coro/scheduler.hpp>
#include <cth/coro/executor.hpp>
#include <cth/coro/tasks/executor_task.hpp>
#include <cth/coro/this_coro.hpp>

#include <atomic>
#include <latch>
#include <thread>
#include <memory>
#include <vector>
#include <stdexcept>

namespace cth::co {

template<class Task>
auto sync_wait(executor& exec, Task task) {
    using value_type = typename Task::promise_type::value_type;

    auto wrapper = [](executor& exec, Task t) -> sync_task<value_type> {
        if constexpr(std::is_void_v<value_type>) { co_await exec.spawn(std::move(t)); } else {
            co_return co_await exec.spawn(std::move(t));
        }
    };

    auto wrapped = wrapper(exec, std::move(task));
    wrapped.handle().resume();

    wrapped.wait();

    if constexpr(!std::is_void_v<value_type>) { return std::move(*wrapped.handle().promise().result); }
}

CORO_TEST(executor, spawn_void_task) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    std::atomic<bool> ran{false};
    auto task = [](std::atomic<bool>& r) -> executor_task<void> {
        r = true;
        co_return;
    };

    sync_wait(exec, task(ran));
    EXPECT_TRUE(ran);
}

CORO_TEST(executor, spawn_value_task) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<int> { co_return 42; };

    int result = sync_wait(exec, task());
    EXPECT_EQ(result, 42);
}

CORO_TEST(executor, spawn_move_only_type) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<std::unique_ptr<int>> { co_return std::make_unique<int>(123); };

    auto result = sync_wait(exec, task());
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(*result, 123);
}

CORO_TEST(executor, spawn_execution_context) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto main_thread = std::this_thread::get_id();

    auto task = []() -> executor_task<std::thread::id> { co_return std::this_thread::get_id(); };

    auto worker_thread = sync_wait(exec, task());

    EXPECT_NE(main_thread, worker_thread);
}

CORO_TEST(executor, context_this_coro_executor) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<executor*> { co_return &co_await this_coro::executor; };

    auto* ptr = sync_wait(exec, task());
    EXPECT_EQ(ptr, &exec);
}

CORO_TEST(executor, context_nested_task_propagation) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto child = []() -> executor_task<executor*> { co_return &co_await this_coro::executor; };

    auto root = [](auto c) -> executor_task<executor*> { co_return co_await c(); };

    auto* ptr = sync_wait(exec, root(child));
    EXPECT_EQ(ptr, &exec);
}

CORO_TEST(executor, context_deep_recursion) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    struct Recursive {
        auto operator()(int depth) -> executor_task<executor*> {
            if(depth == 0) { co_return &co_await this_coro::executor; }
            co_return co_await (*this)(depth - 1);
        }
    };

    auto* ptr = sync_wait(exec, Recursive{}(10));
    EXPECT_EQ(ptr, &exec);
}

struct foreign_task {
    struct awaiter {
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> h) { std::thread([h] { h.resume(); }).detach(); }
        void await_resume() {}
    };
    awaiter operator co_await() { return {}; }
};

struct foreign_value_task {
    int val;
    struct awaiter {
        int v;
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> h) { std::thread([h] { h.resume(); }).detach(); }
        int await_resume() { return v; }
    };
    awaiter operator co_await() { return {val}; }
};

CORO_TEST(executor, interop_steal_foreign_void) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto main_id = std::this_thread::get_id();

    auto task = [](std::thread::id mid) -> executor_task<void> {
        auto id1 = std::this_thread::get_id();
        co_await foreign_task{};
        auto id2 = std::this_thread::get_id();

        if(id1 == mid) throw std::runtime_error("Started on main thread");
        if(id1 != id2) throw std::runtime_error("Did not return to executor thread");
    };

    sync_wait(exec, task(main_id));
}

CORO_TEST(executor, interop_steal_foreign_value) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<int> { co_return co_await foreign_value_task{99}; };

    int res = sync_wait(exec, task());
    EXPECT_EQ(res, 99);
}

CORO_TEST(executor, interop_explicit_steal_injection) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<executor*> { co_return &co_await this_coro::executor; };

    auto launcher = [](executor& e, auto t) -> executor_task<executor*> { co_return co_await e.steal(t()); };

    executor* captured = sync_wait(exec, launcher(exec, task));

    EXPECT_EQ(captured, &exec);
}

CORO_TEST(executor, interop_cross_executor_migration) {
    scheduler sched{autostart, 2};
    executor exec1{sched};
    executor exec2{sched};

    auto sub_task = []() -> executor_task<executor*> { co_return &co_await this_coro::executor; };

    auto root_task = [](executor& e1, executor& e2, auto st) -> executor_task<executor*> {
        auto* e = &co_await this_coro::executor;
        if(e != &e1) co_return nullptr;

        co_return co_await e2.steal(st());
    };

    auto* result = sync_wait(exec1, root_task(exec1, exec2, sub_task));
    EXPECT_EQ(result, &exec2);
}

CORO_TEST(executor, errors_exception_in_spawned_task) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<void> {
        throw std::runtime_error("boom");
        co_return;
    };

    auto wrapper = [](auto t) -> executor_task<bool> {
        bool caught = false;
        try { co_await t(); } catch(std::runtime_error const&) { caught = true; }
        co_return caught;
    };

    bool caught = sync_wait(exec, wrapper(task));
    EXPECT_TRUE(caught);
}

CORO_TEST(executor, errors_exception_in_nested_task) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto child = []() -> executor_task<void> {
        throw std::logic_error("nested boom");
        co_return;
    };

    auto root = [](auto c) -> executor_task<void> { co_await c(); };

    auto wrapper = [](auto r, auto ch) -> executor_task<bool> {
        bool caught = false;
        try { co_await r(ch); } catch(std::logic_error const&) { caught = true; }
        co_return caught;
    };

    bool caught = sync_wait(exec, wrapper(root, child));
    EXPECT_TRUE(caught);
}

CORO_TEST(executor, stress_concurrent_spawns) {
    scheduler sched{autostart, 4};
    executor exec{sched};

    constexpr int count = 100;
    std::vector<scheduled_task<void>> tasks;
    tasks.reserve(count);

    std::atomic<int> completed{0};
    std::latch all_done{count};

    auto increment_task = [&](std::atomic<int>& ctr) -> executor_task<void> { //ref capture is fine bc no co_await
        ctr.fetch_add(1, std::memory_order_relaxed);
        all_done.count_down();
        co_return;
    };

    auto spawner = [&](std::vector<scheduled_task<void>>& t) -> executor_task<void> { //ref capture fine bc no co_await
        for(int i = 0; i < count; ++i) {
            auto h = exec.spawn(increment_task(completed));
            h.handle().resume();
            t.push_back(std::move(h));
        }

        all_done.wait();
        co_return;
    };

    sync_wait(exec, spawner(tasks));

    EXPECT_EQ(completed.load(), count);
}

}
