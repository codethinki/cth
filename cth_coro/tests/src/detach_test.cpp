#include "test.hpp"

#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/simple_counting_scope.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/this_coro.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>


namespace cth::co {
namespace {
    // counts its destructions, the frame of a detached task holds one
    struct destruction_counter {
        explicit destruction_counter(std::atomic<size_t>& count) : _count{&count} {}
        ~destruction_counter() { _count->fetch_add(1); }

        destruction_counter(destruction_counter const&) = delete;
        destruction_counter& operator=(destruction_counter const&) = delete;

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
}
