#include "test.hpp"

#include "cth/win/coro/keyboard.hpp"

#include <cth/coro/executor.hpp>
#include <cth/coro/sync.hpp>
#include <cth/coro/tasks/executor_task.hpp>
#include <cth/coro/this_coro.hpp>

#include <atomic>
#include <chrono>
#include <thread>


namespace cth::win::co {
using namespace std::chrono_literals;

namespace {
    /**
     * the queue's own push is public, so the tests drive it directly instead of needing real key input
     */
    [[nodiscard]] keybd_event_queue::event_t test_event() {
        return keybd_event_queue::event_t{
            .data = ::cth::io::key_state{::cth::io::Key::A, false, true},
            .time = cth::chrono::clock_t::now()
        };
    }

    template<class Task>
    auto sync_wait(cth::co::executor exec, Task task) {
        return cth::co::sync(exec.spawn(std::move(task)));
    }
}


// PROBE: queue lifetime only, no coroutines
CO_TEST(probe, queue_ctor_dtor_only) {
    keybd_event_queue queue{};
}

// PROBE: queue lifetime + push, no coroutines
CO_TEST(probe, queue_push_only) {
    keybd_event_queue queue{};
    queue.push(test_event());
}

CO_TEST(event_queue, exposes_queue_api) {
    keybd_event_queue queue{};

    EXPECT_TRUE(queue.empty());
    queue.push(test_event());
    EXPECT_EQ(queue.size(), 1);
    [[maybe_unused]] auto const front = queue.front();
    EXPECT_TRUE(queue.peek().has_value());
    [[maybe_unused]] auto const popped = queue.pop();
    EXPECT_TRUE(queue.empty());

    queue.push(test_event());
    queue.push(test_event());
    EXPECT_EQ(queue.pop_queue().size(), 2);

    queue.push(test_event());
    queue.clear();
    EXPECT_TRUE(queue.empty());
}

// PROBE: scheduler only, no queue
CO_TEST(probe, scheduler_only) {
    cth::co::scheduler sched{cth::co::autostart, 1};
    cth::co::executor exec{sched};

    auto task = []() -> cth::co::executor_task<int> { co_return 1; };
    EXPECT_EQ(sync_wait(exec, task()), 1);
}

// already queued -> await_ready fast path, no suspension
CO_TEST(event_queue, ready_when_already_queued) {
    cth::co::scheduler sched{cth::co::autostart, 1};
    cth::co::executor exec{sched};

    keybd_event_queue queue{};
    queue.push(test_event());

    auto task = [](keybd_event_queue& q) -> cth::co::executor_task<bool> {
        [[maybe_unused]] auto const event = co_await q.next();
        co_return true;
    };

    EXPECT_TRUE(sync_wait(exec, task(queue)));
}

// the flag makes this deterministic: it is set immediately before the push, so observing it false would
// mean the await resumed without ever suspending
CO_TEST(event_queue, suspends_until_pushed) {
    cth::co::scheduler sched{cth::co::autostart, 1};
    cth::co::executor exec{sched};

    keybd_event_queue queue{};
    std::atomic<bool> pushed{false};

    std::jthread const pusher{
        [&queue, &pushed] {
            std::this_thread::sleep_for(25ms);
            pushed = true;
            queue.push(test_event());
        }
    };

    auto task = [](keybd_event_queue& q, std::atomic<bool> const& flag) -> cth::co::executor_task<bool> {
        [[maybe_unused]] auto const event = co_await q.next();
        co_return flag.load();
    };

    EXPECT_TRUE(sync_wait(exec, task(queue, pushed)));
}

// the OS wait must hand the resume to the scheduler, never run it on the pushing thread
CO_TEST(event_queue, resumes_on_scheduler_thread) {
    cth::co::scheduler sched{cth::co::autostart, 1};
    cth::co::executor exec{sched};

    keybd_event_queue queue{};

    auto const caller = std::this_thread::get_id();
    std::atomic<std::thread::id> pusherId{};

    std::jthread const pusher{
        [&queue, &pusherId] {
            pusherId = std::this_thread::get_id();
            std::this_thread::sleep_for(25ms);
            queue.push(test_event());
        }
    };

    auto task = [](keybd_event_queue& q) -> cth::co::executor_task<std::thread::id> {
        [[maybe_unused]] auto const event = co_await q.next();
        co_return std::this_thread::get_id();
    };

    auto const resumed = sync_wait(exec, task(queue));

    EXPECT_NE(resumed, caller);
    EXPECT_NE(resumed, pusherId.load());
}

// one notification resumes the waiter; the remaining burst stays buffered
CO_TEST(event_queue, survives_burst) {
    cth::co::scheduler sched{cth::co::autostart, 1};
    cth::co::executor exec{sched};

    keybd_event_queue queue{};

    std::jthread const pusher{
        [&queue] {
            std::this_thread::sleep_for(25ms);
            for(int i = 0; i < 16; ++i)
                queue.push(test_event());
        }
    };

    auto task = [](keybd_event_queue& q) -> cth::co::executor_task<size_t> {
        size_t taken = 0;

        while(taken < 16) {
            [[maybe_unused]] auto const event = co_await q.next();
            ++taken;
        }

        co_return taken;
    };

    EXPECT_EQ(sync_wait(exec, task(queue)), 16);
}

}
