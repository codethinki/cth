#include "cth/coro/tasks/sync_task.hpp"
#include "test.hpp"

#include <cth/coro/executor.hpp>
#include <cth/coro/scheduler.hpp>
#include <cth/coro/tasks/executor_task.hpp>
#include <cth/coro/this_coro.hpp>

#include <atomic>
#include <memory>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>


namespace cth::co {

template<class Task>
auto sync_wait(executor& exec, Task task) {
    using value_type = Task::promise_type::value_type;

    auto wrapper = [](executor& exec, Task t) -> sync_task<value_type> {
        if constexpr(mta::is_void<value_type>)
            co_await exec.spawn(std::move(t));
        else
            co_return co_await exec.spawn(std::move(t));
    };

    auto wrapped = wrapper(exec, std::move(task));
    wrapped.handle().resume();

    wrapped.wait();

    if constexpr(!mta::is_void<value_type>)
        return std::move(*wrapped.handle().promise().result);
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

    int const result = sync_wait(exec, task());
    EXPECT_EQ(result, 42);
}

CORO_TEST(executor, spawn_move_only_type) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<std::unique_ptr<int>> {
        co_await this_coro::wait(std::chrono::microseconds{200});

        co_return std::make_unique<int>(123);
    };

    auto const result = sync_wait(exec, task());
    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(*result, 123);
}

CORO_TEST(executor, spawn_execution_context) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto const mainThread = std::this_thread::get_id();

    auto task = []() -> executor_task<std::thread::id> { co_return std::this_thread::get_id(); };

    auto const workerThread = sync_wait(exec, task());

    EXPECT_NE(mainThread, workerThread);
}

CORO_TEST(executor, context_this_coro_executor) {
    scheduler sched{autostart, 1};
    executor expected{sched};

    auto task = []() -> executor_task<executor> { co_return co_await this_coro::executor; };

    auto const actual = sync_wait(expected, task());
    EXPECT_EQ(actual, expected);
}

CORO_TEST(executor, context_nested_task_propagation) {
    scheduler sched{autostart, 1};
    executor expected{sched};

    auto child = []() -> executor_task<executor> { co_return co_await this_coro::executor; };

    auto root = [](auto c) -> executor_task<executor> { co_return co_await c(); };

    auto const actual = sync_wait(expected, root(child));
    EXPECT_EQ(actual, expected);
}

CORO_TEST(executor, context_deep_recursion) {
    scheduler sched{autostart, 1};
    executor expected{sched};

    struct Recursive {
        auto operator()(int depth) -> executor_task<executor> {
            if(depth == 0) {
                co_return co_await this_coro::executor;
            }
            co_return co_await (*this)(depth - 1);
        }
    };

    auto const actual = sync_wait(expected, Recursive{}(10));
    EXPECT_EQ(actual, expected);
}

struct foreign_task {
    struct awaiter {
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> h) {
            std::thread([h] { h.resume(); }).detach();
        }
        void await_resume() {}
    };
    awaiter operator co_await() { return {}; }
};

struct foreign_value_task {
    int val;
    struct awaiter {
        int v;
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> h) {
            std::thread([h] { h.resume(); }).detach();
        }
        int await_resume() { return v; }
    };
    awaiter operator co_await() { return {val}; }
};

// completes on the target scheduler, throws on resume
struct foreign_throwing_awaiter {
    scheduler const& target;

    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) const {
        target.post([h] { h.resume(); });
    }
    [[noreturn]] void await_resume() const { throw std::runtime_error("foreign boom"); }
};

CORO_TEST(executor, interop_steal_foreign_void) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto const mainID = std::this_thread::get_id();

    auto task = [](std::thread::id mid) -> executor_task<void> {
        auto const id1 = std::this_thread::get_id();
        co_await foreign_task{};
        auto const id2 = std::this_thread::get_id();

        if(id1 == mid)
            throw std::runtime_error("Started on main thread");
        if(id1 != id2)
            throw std::runtime_error("Did not return to executor thread");
    };

    sync_wait(exec, task(mainID));
}

CORO_TEST(executor, interop_steal_foreign_value) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<int> { co_return co_await foreign_value_task{99}; };

    int const res = sync_wait(exec, task());
    EXPECT_EQ(res, 99);
}

// after schedule() onto another scheduler, this_coro::executor reports the target. Two schedulers: executor
// equality compares the scheduler pointer, executors on one scheduler compare equal and prove nothing.
CORO_TEST(executor, context_reflects_schedule_hop) {
    scheduler home{autostart, 1};
    scheduler other{autostart, 1};
    executor homeExec{home};
    executor otherExec{other};

    auto task = [](executor target) -> executor_task<executor> {
        co_await target.schedule();             // physically hop onto `target`
        co_return co_await this_coro::executor; // ask "which executor am I on?"
    };

    auto const actual = sync_wait(homeExec, task(otherExec));
    EXPECT_EQ(actual, otherExec);
}

// the same through the scheduler tag: after schedule() this_coro::scheduler names the target, not the
// scheduler of the spawn.
CORO_TEST(executor, context_scheduler_reflects_schedule_hop) {
    scheduler home{autostart, 1};
    scheduler other{autostart, 1};
    executor homeExec{home};
    executor otherExec{other};

    auto task = [](executor target) -> executor_task<scheduler const*> {
        co_await target.schedule();
        scheduler const& current = co_await this_coro::scheduler;
        co_return &current;
    };

    auto const* actual = sync_wait(homeExec, task(otherExec));
    EXPECT_EQ(actual, &other);
}

// a child that moves to another scheduler continues the parent on the scheduler of the parent
CORO_TEST(executor, context_parent_continues_on_own_scheduler_after_child_schedule) {
    scheduler home{autostart, 1};
    scheduler other{autostart, 1};
    executor homeExec{home};

    auto child = [](executor target) -> executor_task<bool> {
        co_await target.schedule();
        co_return target.scheduler().owns_thread();
    };

    auto task = [](scheduler const& h, executor o, auto c)
        -> executor_task<std::tuple<bool, bool, scheduler const*>> {
        bool const childOnOther = co_await c(o);
        scheduler const& current = co_await this_coro::scheduler;
        co_return std::tuple{childOnOther, h.owns_thread(), &current};
    };

    auto const [childOnOther, onHome, current] = sync_wait(homeExec, task(home, executor{other}, child));
    EXPECT_TRUE(childOnOther);
    EXPECT_TRUE(onHome);
    EXPECT_EQ(current, &home);
}

// spawn() runs the child on its scheduler, the caller continues on its own scheduler afterwards
CORO_TEST(executor, interop_spawn_caller_continues_on_own_scheduler) {
    scheduler home{autostart, 1};
    scheduler other{autostart, 1};
    executor homeExec{home};

    auto child = [](scheduler const& target) -> executor_task<bool> { co_return target.owns_thread(); };

    auto task = [](scheduler const& h, scheduler const& o, auto c)
        -> executor_task<std::tuple<bool, bool, scheduler const*>> {
        bool const childOnOther = co_await executor{o}.spawn(c(o));
        scheduler const& current = co_await this_coro::scheduler;
        co_return std::tuple{childOnOther, h.owns_thread(), &current};
    };

    auto const [childOnOther, onHome, current] = sync_wait(homeExec, task(home, other, child));
    EXPECT_TRUE(childOnOther);
    EXPECT_TRUE(onHome);
    EXPECT_EQ(current, &home);
}

CORO_TEST(executor, interop_spawn_on_own_scheduler) {
    scheduler home{autostart, 1};
    executor homeExec{home};

    auto child = [](scheduler const& target) -> executor_task<bool> { co_return target.owns_thread(); };

    auto task = [](executor e, scheduler const& h, auto c) -> executor_task<std::pair<bool, bool>> {
        bool const childOnHome = co_await e.spawn(c(h));
        co_return std::pair{childOnHome, h.owns_thread()};
    };

    auto const [childOnHome, onHome] = sync_wait(homeExec, task(homeExec, home, child));
    EXPECT_TRUE(childOnHome);
    EXPECT_TRUE(onHome);
}

CORO_TEST(executor, interop_spawn_void_caller_continues_on_own_scheduler) {
    scheduler home{autostart, 1};
    scheduler other{autostart, 1};
    executor homeExec{home};
    std::atomic<bool> childOnOther{false};

    auto child = [](scheduler const& target, std::atomic<bool>& flag) -> executor_task<void> {
        flag = target.owns_thread();
        co_return;
    };

    auto task = [](scheduler const& h, scheduler const& o, std::atomic<bool>& flag, auto c) -> executor_task<bool> {
        co_await executor{o}.spawn(c(o, flag));
        co_return h.owns_thread();
    };

    EXPECT_TRUE(sync_wait(homeExec, task(home, other, childOnOther, child)));
    EXPECT_TRUE(childOnOther);
}

// the child may end on another scheduler than the one it was spawned on
CORO_TEST(executor, interop_spawn_child_ends_elsewhere) {
    scheduler home{autostart, 1};
    scheduler other{autostart, 1};
    scheduler third{autostart, 1};
    executor homeExec{home};

    auto child = [](executor end) -> executor_task<void> { co_await end.schedule(); };

    auto task = [](scheduler const& h, executor o, executor t, auto c) -> executor_task<std::pair<bool, bool>> {
        co_await o.spawn(c(t));
        bool const afterThird = h.owns_thread();

        co_await o.spawn(c(executor{h}));
        co_return std::pair{afterThird, h.owns_thread()};
    };

    auto const [afterThird, afterHome] = sync_wait(homeExec, task(home, executor{other}, executor{third}, child));
    EXPECT_TRUE(afterThird);
    EXPECT_TRUE(afterHome);
}

CORO_TEST(executor, interop_spawn_exception_reaches_caller_on_own_scheduler) {
    scheduler home{autostart, 1};
    scheduler other{autostart, 1};
    executor homeExec{home};

    auto child = []() -> executor_task<void> {
        throw std::runtime_error("spawned boom");
        co_return;
    };

    auto task = [](scheduler const& h, executor o, auto c) -> executor_task<std::pair<bool, bool>> {
        bool caught = false;
        try {
            co_await o.spawn(c());
        } catch(std::runtime_error const&) {
            caught = true;
        }
        co_return std::pair{caught, h.owns_thread()};
    };

    auto const [caught, onHome] = sync_wait(homeExec, task(home, executor{other}, child));
    EXPECT_TRUE(caught);
    EXPECT_TRUE(onHome);
}

CORO_TEST(executor, interop_spawn_nested_each_level_on_own_scheduler) {
    scheduler home{autostart, 1};
    scheduler middle{autostart, 1};
    scheduler inner{autostart, 1};
    executor homeExec{home};

    auto grandchild = [](scheduler const& i) -> executor_task<bool> { co_return i.owns_thread(); };

    auto child = [](scheduler const& m, scheduler const& i, auto g) -> executor_task<std::pair<bool, bool>> {
        bool const grandchildOnInner = co_await executor{i}.spawn(g(i));
        co_return std::pair{grandchildOnInner, m.owns_thread()};
    };

    auto task = [](scheduler const& h, scheduler const& m, scheduler const& i, auto c, auto g)
        -> executor_task<std::tuple<bool, bool, bool>> {
        auto const [grandchildOnInner, childOnMiddle] = co_await executor{m}.spawn(c(m, i, g));
        co_return std::tuple{grandchildOnInner, childOnMiddle, h.owns_thread()};
    };

    auto const [grandchildOnInner, childOnMiddle, onHome] =
        sync_wait(homeExec, task(home, middle, inner, child, grandchild));
    EXPECT_TRUE(grandchildOnInner);
    EXPECT_TRUE(childOnMiddle);
    EXPECT_TRUE(onHome);
}

// a caller without a scheduler continues on the scheduler of the executor, also if the child ends elsewhere
CORO_TEST(executor, interop_spawn_caller_without_scheduler_continues_on_executor_scheduler) {
    scheduler other{autostart, 1};
    scheduler third{autostart, 1};

    auto child = [](executor end) -> executor_task<bool> {
        co_await end.schedule();
        co_return end.scheduler().owns_thread();
    };

    auto task = [](scheduler const& o, executor t, auto c) -> sync_task<std::pair<bool, bool>> {
        bool const childOnThird = co_await executor{o}.spawn(c(t));
        co_return std::pair{childOnThird, o.owns_thread()};
    };

    auto waiter = task(other, executor{third}, child);
    waiter.start();
    auto const [childOnThird, onOther] = waiter.await();
    EXPECT_TRUE(childOnThird);
    EXPECT_TRUE(onOther);
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
        try {
            co_await t();
        } catch(std::runtime_error const&) {
            caught = true;
        }
        co_return caught;
    };

    bool const caught = sync_wait(exec, wrapper(task));
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
        try {
            co_await r(ch);
        } catch(std::logic_error const&) {
            caught = true;
        }
        co_return caught;
    };

    bool const caught = sync_wait(exec, wrapper(root, child));
    EXPECT_TRUE(caught);
}

// a child that throws after schedule() onto another scheduler: the parent catches on its own scheduler
CORO_TEST(executor, errors_parent_catches_on_own_scheduler_after_child_schedule) {
    scheduler home{autostart, 1};
    scheduler other{autostart, 1};
    executor homeExec{home};

    auto child = [](executor target) -> executor_task<void> {
        co_await target.schedule();
        throw std::runtime_error("child boom");
    };

    auto task = [](scheduler const& h, executor o, auto c)
        -> executor_task<std::tuple<bool, bool, scheduler const*>> {
        bool caughtOnHome = false;
        try {
            co_await c(o);
        } catch(std::runtime_error const&) {
            caughtOnHome = h.owns_thread();
        }
        scheduler const& current = co_await this_coro::scheduler;
        co_return std::tuple{caughtOnHome, h.owns_thread(), &current};
    };

    auto const [caughtOnHome, onHome, current] = sync_wait(homeExec, task(home, executor{other}, child));
    EXPECT_TRUE(caughtOnHome);
    EXPECT_TRUE(onHome);
    EXPECT_EQ(current, &home);
}

// a foreign awaitable that completes on another scheduler and throws: the caller catches on its own scheduler
CORO_TEST(executor, errors_foreign_exception_reaches_caller_on_own_scheduler) {
    scheduler home{autostart, 1};
    scheduler other{autostart, 1};
    executor homeExec{home};

    auto task = [](scheduler const& h, scheduler const& o) -> executor_task<bool> {
        bool caughtOnHome = false;
        try {
            co_await foreign_throwing_awaiter{o};
        } catch(std::runtime_error const&) {
            caughtOnHome = h.owns_thread();
        }
        co_return caughtOnHome;
    };

    EXPECT_TRUE(sync_wait(homeExec, task(home, other)));
}

CORO_TEST(executor, stress_concurrent_spawns) {
    scheduler sched{autostart, 4};
    executor exec{sched};

    constexpr int count = 100;
    std::atomic<int> completed{0};

    auto increment_task = [](std::atomic<int>& ctr) -> executor_task<void> {
        ctr.fetch_add(1, std::memory_order_relaxed);
        co_return;
    };

    // starts the spawn now, waits later
    auto start = [](spawn_task<void> t) -> sync_task<void> { co_await t; };

    std::vector<sync_task<void>> tasks;
    tasks.reserve(count);
    for(int i = 0; i < count; ++i) {
        tasks.push_back(start(exec.spawn(increment_task(completed))));
        tasks.back().start();
    }

    for(auto const& task : tasks)
        task.wait();

    EXPECT_EQ(completed.load(), count);
}

}
