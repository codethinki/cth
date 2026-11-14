#include "test.hpp"

#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/simple_counting_scope.hpp"
#include "cth/coro/strand.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/this_coro.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <thread>
#include <utility>


namespace cth::co {
namespace {
    // counts its destructions, the frame of a detached task holds one
    // the slow destruction: a wait() that returns before the end of the destruction sees the old count
    struct destruction_counter {
        explicit destruction_counter(std::atomic<size_t>& count) : _count{&count} {}
        ~destruction_counter() {
            if(_count == nullptr) return;
            std::this_thread::sleep_for(std::chrono::milliseconds{1});
            _count->fetch_add(1);
        }

        // a coroutine moves its parameters into its frame, a moved from counter counts nothing
        destruction_counter(destruction_counter&& other) noexcept : _count{std::exchange(other._count, nullptr)} {}
        destruction_counter(destruction_counter const&) = delete;
        destruction_counter& operator=(destruction_counter const&) = delete;
        destruction_counter& operator=(destruction_counter&&) = delete;

    private:
        std::atomic<size_t>* _count;
    };

    // a payload with a marker value
    struct marked_payload : this_coro::payload_base {
        constexpr explicit marked_payload(int marker) : _marker{marker} {}

        [[nodiscard]] constexpr int marker() const { return _marker; }

    private:
        int _marker;
    };

    struct marker_tag : this_coro::tag_base {
        static constexpr auto operator()(co::context const&, marked_payload const& p) {
            return data_awaiter{p.marker()};
        }
    };

    inline constexpr marker_tag marker{};
}

CORO_TEST(simple_counting_scope, wait_on_an_empty_scope_returns_at_once) {
    simple_counting_scope const tasks{};

    tasks.wait();
}

CORO_TEST(executor, detach_runs_on_the_context) {
    scheduler sched{autostart, 1};
    executor exec{sched};
    simple_counting_scope tasks{};

    std::atomic<bool> onContext{false};
    auto task = [](scheduler const& s, std::atomic<bool>& result) -> executor_task<void> {
        result = s.owns_thread();
        co_return;
    };

    exec.detach(tasks, task(sched, onContext));
    tasks.wait();

    EXPECT_TRUE(onContext);
}

CORO_TEST(executor, detach_without_a_scope_runs_on_the_context) {
    std::atomic<bool> onContext{false};
    std::atomic<bool> ended{false};
    // after the atomics: the destruction of the scheduler waits for the job of the task
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = [](scheduler const& s, std::atomic<bool>& result, std::atomic<bool>& done) -> executor_task<void> {
        result = s.owns_thread();
        done = true;
        done.notify_one();
        co_return;
    };

    exec.detach(task(sched, onContext, ended));
    ended.wait(false);

    EXPECT_TRUE(onContext);
}

CORO_TEST(executor, detach_without_a_scope_is_in_no_scope) {
    std::atomic<bool> release{false};
    std::atomic<bool> ended{false};
    // after the atomics: the destruction of the scheduler waits for the job of the task
    scheduler sched{autostart, 2};
    executor exec{sched};
    simple_counting_scope tasks{};

    auto blocked = [](std::atomic<bool>& go, std::atomic<bool>& done) -> executor_task<void> {
        go.wait(false);
        done = true;
        done.notify_one();
        co_return;
    };
    auto quick = []() -> executor_task<void> { co_return; };

    exec.detach(blocked(release, ended));
    exec.detach(tasks, quick());

    // the scope counts its own task only, the task without a scope still blocks
    tasks.wait();
    EXPECT_FALSE(ended.load());

    release = true;
    release.notify_one();
    ended.wait(false);
}

CORO_TEST(executor, detach_wait_returns_after_the_frames_are_destroyed) {
    constexpr size_t TASKS = 16;
    scheduler sched{autostart, 2};
    executor exec{sched};
    simple_counting_scope tasks{};

    std::atomic<size_t> destroyed{0};
    auto task = [](std::atomic<size_t>& count) -> executor_task<void> {
        destruction_counter const counter{count};
        // suspends before the end
        co_await this_coro::wait(std::chrono::milliseconds{2});
        co_await this_coro::executor;
    };

    for(size_t i = 0; i < TASKS; ++i)
        exec.detach(tasks, task(destroyed));
    tasks.wait();

    EXPECT_EQ(destroyed.load(), TASKS);
}

CORO_TEST(executor, detach_wait_returns_after_the_parameters_are_destroyed) {
    constexpr size_t TASKS = 4;
    scheduler sched{autostart, 2};
    executor exec{sched};
    simple_counting_scope tasks{};

    std::atomic<size_t> destroyed{0};
    // the frame of the task holds the counter until the frame is destroyed
    auto task = [](destruction_counter) -> executor_task<void> { co_await this_coro::executor; };

    for(size_t i = 0; i < TASKS; ++i)
        exec.detach(tasks, task(destruction_counter{destroyed}));
    tasks.wait();

    EXPECT_EQ(destroyed.load(), TASKS);
}

CORO_TEST(executor, detach_tasks_suspend_and_move_between_contexts) {
    constexpr size_t TASKS = 8;
    scheduler first{autostart, 1};
    scheduler second{autostart, 1};
    executor exec{first};
    simple_counting_scope tasks{};

    std::atomic<size_t> onSecond{0};
    auto task = [](scheduler const& target, std::atomic<size_t>& count) -> executor_task<void> {
        co_await executor{target}.schedule();
        if(target.owns_thread()) count.fetch_add(1);
    };

    for(size_t i = 0; i < TASKS; ++i)
        exec.detach(tasks, task(second, onSecond));
    tasks.wait();

    EXPECT_EQ(onSecond.load(), TASKS);
}

CORO_TEST(executor, detach_passes_the_payload) {
    scheduler sched{autostart, 1};
    executor exec{sched};
    simple_counting_scope tasks{};

    std::atomic<int> received{0};
    auto task = [](std::atomic<int>& result) -> executor_task<void, marked_payload> {
        result = co_await marker;
    };

    exec.detach<marked_payload>(tasks, task(received), 42);
    tasks.wait();

    EXPECT_EQ(received.load(), 42);
}

CORO_TEST(scheduler, destroys_a_detached_task_that_did_not_end) {
    simple_counting_scope tasks{};
    std::atomic<size_t> destroyed{0};
    std::atomic<bool> waits{false};

    {
        scheduler sched{autostart, 1};
        executor exec{sched};

        auto task = [](std::atomic<size_t>& count, std::atomic<bool>& started) -> executor_task<void> {
            destruction_counter const counter{count};
            started = true;
            started.notify_one();
            // does not end before the destruction of the scheduler
            co_await this_coro::wait(std::chrono::hours{1});
        };

        exec.detach(tasks, task(destroyed, waits));
        waits.wait(false);
    }

    ASSERT_EQ(destroyed.load(), 1);
    // returns at once: the destruction left the scope
    tasks.wait();
}

CORO_TEST(scheduler, destroys_a_task_detached_without_a_scope_that_did_not_end) {
    std::atomic<size_t> destroyed{0};
    std::atomic<bool> waits{false};

    {
        scheduler sched{autostart, 1};
        executor exec{sched};

        auto task = [](std::atomic<size_t>& count, std::atomic<bool>& started) -> executor_task<void> {
            destruction_counter const counter{count};
            started = true;
            started.notify_one();
            // does not end before the destruction of the scheduler
            co_await this_coro::wait(std::chrono::hours{1});
        };

        exec.detach(task(destroyed, waits));
        waits.wait(false);
    }

    EXPECT_EQ(destroyed.load(), 1);
}

CORO_TEST(scheduler, destroys_a_task_detached_on_a_strand_that_did_not_end) {
    simple_counting_scope tasks{};
    std::atomic<size_t> destroyed{0};
    std::atomic<bool> waits{false};

    {
        scheduler sched{autostart, 1};
        strand str{sched};

        // waits on the strand: the scheduler stops first, nothing resumes on the strand then
        auto task = [](std::atomic<size_t>& count, std::atomic<bool>& started) -> executor_task<void> {
            destruction_counter const counter{count};
            started = true;
            started.notify_one();
            co_await this_coro::wait(std::chrono::hours{1});
        };

        executor{str}.detach(tasks, task(destroyed, waits));
        waits.wait(false);
        sched.await_stop();
    }

    ASSERT_EQ(destroyed.load(), 1);
    tasks.wait();
}
}
