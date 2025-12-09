#include "test.hpp"
#include "cth/coro/fence.hpp"
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
void sync_wait(executor& exec, Task task) {
    fence done;
    auto wrapper = [](fence& f, Task t) -> executor_task<void> {
        co_await std::move(t);
        f.signal();
    };

    auto handle = exec.spawn(wrapper(done, std::move(task)));
    handle.handle().resume();

    done.wait();
    //BUG here, wait for coroutine to finish
}

template<class T, class Task>
T sync_wait_result(executor& exec, Task task) {
    fence done;
    std::optional<T> result;
    auto wrapper = [](fence& f, std::optional<T>& res, Task t) -> executor_task<void> {
        res = co_await std::move(t);
        f.signal();
    };

    auto handle = exec.spawn(wrapper(done, result, std::move(task)));
    handle.handle().resume();

    done.wait();
    //BUG here, wait for coroutine to finish

    return std::move(*result);
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

    int result = sync_wait_result<int>(exec, task());
    EXPECT_EQ(result, 42);
}

CORO_TEST(executor, spawn_move_only_type) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<std::unique_ptr<int>> { co_return std::make_unique<int>(123); };

    auto result = sync_wait_result<std::unique_ptr<int>>(exec, task());
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(*result, 123);
}

CORO_TEST(executor, spawn_execution_context) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto main_thread = std::this_thread::get_id();

    auto task = []() -> executor_task<std::thread::id> { co_return std::this_thread::get_id(); };

    auto worker_thread = sync_wait_result<std::thread::id>(exec, task());

    EXPECT_NE(main_thread, worker_thread);
}

CORO_TEST(executor, context_this_coro_executor) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<executor*> { co_return &co_await this_coro::executor; };

    auto* ptr = sync_wait_result<executor*>(exec, task());
    EXPECT_EQ(ptr, &exec);
}

CORO_TEST(executor, context_nested_task_propagation) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto child = []() -> executor_task<executor*> { co_return &co_await this_coro::executor; };

    auto root = [](auto c) -> executor_task<executor*> { co_return co_await c(); };

    auto* ptr = sync_wait_result<executor*>(exec, root(child));
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

    auto* ptr = sync_wait_result<executor*>(exec, Recursive{}(10));
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

    int res = sync_wait_result<int>(exec, task());
    EXPECT_EQ(res, 99);
}

CORO_TEST(executor, interop_explicit_steal_injection) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<executor*> { co_return &co_await this_coro::executor; };

    fence done;
    executor* captured = nullptr;

    auto launcher = [](executor& e, executor*& cap, fence& f, auto t) -> executor_task<void> {
        cap = co_await e.steal(t());
        f.signal();
    };

    auto h = exec.spawn(launcher(exec, captured, done, task));
    h.handle().resume();
    done.wait();
    //BUG here, wait for coroutine to finish

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

    auto* result = sync_wait_result<executor*>(exec1, root_task(exec1, exec2, sub_task));
    EXPECT_EQ(result, &exec2);
}

CORO_TEST(executor, errors_exception_in_spawned_task) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<void> {
        throw std::runtime_error("boom");
        co_return;
    };

    fence done;
    bool caught = false;

    auto wrapper = [](fence& f, bool& c, auto t) -> executor_task<void> {
        try { co_await t(); } catch(std::runtime_error const&) { c = true; }
        f.signal();
    };

    auto h = exec.spawn(wrapper(done, caught, task));
    h.handle().resume();
    done.wait();
    //BUG here, wait for coroutine to finish
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

    fence done;
    bool caught = false;

    auto wrapper = [](fence& f, bool& c, auto r, auto ch) -> executor_task<void> {
        try { co_await r(ch); } catch(std::logic_error const&) { c = true; }
        f.signal();
    };

    auto h = exec.spawn(wrapper(done, caught, root, child));
    h.handle().resume();
    done.wait();
    //BUG here, wait for coroutine to finish
    EXPECT_TRUE(caught);
}

CORO_TEST(executor, stress_concurrent_spawns) {
    scheduler sched{autostart, 4};
    executor exec{sched};

    constexpr int count = 100;
    std::vector<scheduled_task<void>> tasks;
    tasks.reserve(count);

    std::atomic<int> completed{0};

    auto increment_task = [](std::atomic<int>& ctr) -> executor_task<void> {
        ctr.fetch_add(1, std::memory_order_relaxed);
        co_return;
    };

    for(int i = 0; i < count; ++i) {
        auto h = exec.spawn(increment_task(completed));
        h.handle().resume();
        tasks.push_back(std::move(h));
    }

    fence done;

    auto waiter = [](std::vector<scheduled_task<void>>& t, fence& f) -> executor_task<void> {
        for(auto& h : t) { co_await std::move(h); }
        f.signal();
    };

    auto h_waiter = exec.spawn(waiter(tasks, done));
    h_waiter.handle().resume();

    done.wait();
    //BUG here, wait for coroutine to finish

    EXPECT_EQ(completed.load(), count);
}

}