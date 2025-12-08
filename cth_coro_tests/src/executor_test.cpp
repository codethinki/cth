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

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

// Helper to run a void task and block until completion
// Note: spawn() is lazy, so we must manually resume the returned handle to kick it off.
template<class T>
void sync_wait(executor& exec, T&& task) {
    fence done;
    auto wrapper = [&]() -> executor_task<void> {
        co_await std::forward<T>(task);
        done.signal();
    };

    auto handle = exec.spawn(wrapper());
    // Kick off the lazy scheduled_task. 
    // It will run until the first suspend (schedule()), post to scheduler, and return.
    handle.handle().resume();

    done.wait();
}

// Helper to run a task returning T and return the value synchronously
template<class T, class Task>
T sync_wait_result(executor& exec, Task&& task) {
    fence done;
    std::optional<T> result;
    auto wrapper = [&]() -> executor_task<void> {
        result = co_await std::forward<Task>(task);
        done.signal();
    };

    auto handle = exec.spawn(wrapper());
    handle.handle().resume();

    done.wait();
    return std::move(*result);
}

// -----------------------------------------------------------------------------
// Suite 1: Basic Lifecycle & Spawn
// -----------------------------------------------------------------------------

CORO_TEST(executor_spawn, void_task) {
    scheduler sched{1};
    executor exec{sched};
    sched.start();

    std::atomic<bool> ran{false};
    auto task = [&]() -> executor_task<void> {
        ran = true;
        co_return;
    };

    sync_wait(exec, task());
    EXPECT_TRUE(ran);
}

CORO_TEST(executor_spawn, value_task) {
    scheduler sched{1};
    executor exec{sched};
    sched.start();

    auto task = []() -> executor_task<int> { co_return 42; };

    int result = sync_wait_result<int>(exec, task());
    EXPECT_EQ(result, 42);
}

CORO_TEST(executor_spawn, move_only_type) {
    scheduler sched{1};
    executor exec{sched};
    sched.start();

    auto task = []() -> executor_task<std::unique_ptr<int>> { co_return std::make_unique<int>(123); };

    auto result = sync_wait_result<std::unique_ptr<int>>(exec, task());
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(*result, 123);
}

CORO_TEST(executor_spawn, execution_context) {
    scheduler sched{1};
    executor exec{sched};
    sched.start();

    auto main_thread = std::this_thread::get_id();

    auto task = []() -> executor_task<std::thread::id> { co_return std::this_thread::get_id(); };

    auto worker_thread = sync_wait_result<std::thread::id>(exec, task());
    EXPECT_NE(main_thread, worker_thread);
}

// -----------------------------------------------------------------------------
// Suite 2: Context Propagation
// -----------------------------------------------------------------------------

CORO_TEST(executor_context, this_coro_executor) {
    scheduler sched{1};
    executor exec{sched};
    sched.start();
    //BUG forgot to implement the tag transform xD needs to be fixed
    auto task = [&]() -> executor_task<executor*> { co_return &co_await this_coro::executor; };

    auto* ptr = sync_wait_result<executor*>(exec, task());
    EXPECT_EQ(ptr, &exec);
}

CORO_TEST(executor_context, nested_task_propagation) {
    scheduler sched{1};
    executor exec{sched};
    sched.start();

    auto child = [&]() -> executor_task<executor*> { co_return &co_await this_coro::executor; };

    auto root = [&]() -> executor_task<executor*> { co_return co_await child(); };

    auto* ptr = sync_wait_result<executor*>(exec, root());
    EXPECT_EQ(ptr, &exec);
}

CORO_TEST(executor_context, deep_recursion) {
    scheduler sched{1};
    executor exec{sched};
    sched.start();

    struct Recursive {
        auto operator()(int depth) -> executor_task<executor*> {
            if(depth == 0) { co_return &co_await this_coro::executor; }
            co_return co_await (*this)(depth - 1);
        }
    };

    auto* ptr = sync_wait_result<executor*>(exec, Recursive{}(10));
    EXPECT_EQ(ptr, &exec);
}

// -----------------------------------------------------------------------------
// Suite 3: Stealing & Interop
// -----------------------------------------------------------------------------

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

CORO_TEST(executor_interop, steal_foreign_void) {
    scheduler sched{1};
    executor exec{sched};
    sched.start();

    auto main_id = std::this_thread::get_id();

    auto task = [&]() -> executor_task<void> {
        auto id1 = std::this_thread::get_id();
        co_await foreign_task{};
        auto id2 = std::this_thread::get_id();

        if(id1 == main_id) throw std::runtime_error("Started on main thread");
        if(id1 != id2) throw std::runtime_error("Did not return to executor thread");
    };

    sync_wait(exec, task());
}

CORO_TEST(executor_interop, steal_foreign_value) {
    scheduler sched{1};
    executor exec{sched};
    sched.start();

    auto task = []() -> executor_task<int> { co_return co_await foreign_value_task{99}; };

    int res = sync_wait_result<int>(exec, task());
    EXPECT_EQ(res, 99);
}

CORO_TEST(executor_interop, explicit_steal_injection) {
    scheduler sched{1};
    executor exec{sched};
    sched.start();

    auto task = [&]() -> executor_task<executor*> { co_return &co_await this_coro::executor; };

    fence done;
    executor* captured = nullptr;

    auto launcher = [&]() -> executor_task<void> {
        captured = co_await exec.steal(task());
        done.signal();
    };

    auto h = exec.spawn(launcher());
    h.handle().resume();
    done.wait();

    EXPECT_EQ(captured, &exec);
}

CORO_TEST(executor_interop, cross_executor_migration) {
    scheduler sched{2};
    executor exec1{sched};
    executor exec2{sched};
    sched.start();

    auto sub_task = [&]() -> executor_task<executor*> { co_return &co_await this_coro::executor; };

    auto root_task = [&]() -> executor_task<executor*> {
        auto* e1 = &co_await this_coro::executor;
        if(e1 != &exec1) co_return nullptr;

        co_return co_await exec2.steal(sub_task());
    };

    auto* result = sync_wait_result<executor*>(exec1, root_task());
    EXPECT_EQ(result, &exec2);
}

// -----------------------------------------------------------------------------
// Suite 4: Error Handling
// -----------------------------------------------------------------------------

CORO_TEST(executor_errors, exception_in_spawned_task) {
    scheduler sched{1};
    executor exec{sched};
    sched.start();

    auto task = []() -> executor_task<void> {
        throw std::runtime_error("boom");
        co_return;
    };

    fence done;
    bool caught = false;
    auto wrapper = [&]() -> executor_task<void> {
        try { co_await task(); } catch(std::runtime_error const&) { caught = true; }
        done.signal();
    };

    auto h = exec.spawn(wrapper());
    h.handle().resume();
    done.wait();
    EXPECT_TRUE(caught);
}

CORO_TEST(executor_errors, exception_in_nested_task) {
    scheduler sched{1};
    executor exec{sched};
    sched.start();

    auto child = []() -> executor_task<void> {
        throw std::logic_error("nested boom");
        co_return;
    };

    auto root = [&]() -> executor_task<void> { co_await child(); };

    fence done;
    bool caught = false;
    auto wrapper = [&]() -> executor_task<void> {
        try { co_await root(); } catch(std::logic_error const&) { caught = true; }
        done.signal();
    };

    auto h = exec.spawn(wrapper());
    h.handle().resume();
    done.wait();
    EXPECT_TRUE(caught);
}

// -----------------------------------------------------------------------------
// Suite 5: Stress & Concurrency
// -----------------------------------------------------------------------------

CORO_TEST(executor_stress, concurrent_spawns) {
    scheduler sched{4};
    executor exec{sched};
    sched.start();

    constexpr int count = 100;
    std::vector<scheduled_task<void>> tasks;
    tasks.reserve(count);

    std::atomic<int> completed{0};

    // Spawn many tasks
    for(int i = 0; i < count; ++i) {
        auto h = exec.spawn(
            [&]() -> executor_task<void> {
                completed.fetch_add(1, std::memory_order_relaxed);
                co_return;
            }()
        );
        // Kick off immediately
        h.handle().resume();
        tasks.push_back(std::move(h));
    }

    // Wait for all of them
    fence done;
    auto waiter = [&]() -> executor_task<void> {
        for(auto& h : tasks) { co_await std::move(h); }
        done.signal();
    };

    auto h_waiter = exec.spawn(waiter());
    h_waiter.handle().resume();

    done.wait();

    EXPECT_EQ(completed.load(), count);
}

} // namespace cth::co
