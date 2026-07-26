#include "test.hpp"

#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/this_coro.hpp"

#include <atomic>
#include <chrono>
#include <thread>

// last: pulls in Windows.h, which leaks min/max macros into everything below it
#include "util/os_test_utilty.hpp"


namespace cth::co::this_coro {
// AI

namespace {
    /**
     * RAII wrapper around the raw test handle, keeps the tests cross platform
     * (manual reset win32 event / eventfd)
     */
    struct scoped_event {
        scoped_event() : handle{os::create_event()} {}
        ~scoped_event() { os::close_handle(handle); }

        void signal() const { os::signal_event(handle); }

        os::native_handle_t handle;

        scoped_event(scoped_event const&) = delete;
        scoped_event& operator=(scoped_event const&) = delete;
        scoped_event(scoped_event&&) = delete;
        scoped_event& operator=(scoped_event&&) = delete;
    };

    /**
     * the task factories below take everything by parameter instead of capturing: a capturing closure is
     * a temporary that dies at the end of the call expression, while the coroutine outlives it.
     */
    auto await_tag_task(os::native_handle_t h, std::atomic<bool> const& flag) -> executor_task<bool> {
        co_await native_handle_tag{h};
        co_return flag.load();
    }

    auto await_handle_task(os::native_handle_t h, std::atomic<bool> const& flag) -> executor_task<bool> {
        co_await this_coro::await_handle(h);
        co_return flag.load();
    }
}


// an already signaled handle takes the await_ready fast path -- no suspension, no scheduler round trip
TAG_TEST(native_handle_tag, ready_when_already_signaled) {
    co::scheduler sched{autostart, 1};
    co::executor exec{sched};

    scoped_event event{};
    event.signal();

    std::atomic<bool> const unused{true};

    auto const start = chrono::clock_t::now();
    EXPECT_TRUE(sync_wait(exec, await_tag_task(event.handle, unused)));
    auto const end = chrono::clock_t::now();

    EXPECT_LT(end, start + std::chrono::milliseconds{100});
}

// the flag is what makes this deterministic instead of timing based: it is only set immediately before
// the signal, so observing it false would mean the await resumed early, i.e. never suspended at all.
TAG_TEST(native_handle_tag, suspends_until_signaled) {
    co::scheduler sched{autostart, 1};
    co::executor exec{sched};

    scoped_event event{};
    std::atomic<bool> signaled{false};

    // declared after event => joined before the handle closes
    std::jthread const signaller{
        [&event, &signaled] {
            std::this_thread::sleep_for(std::chrono::milliseconds{25});
            signaled = true;
            event.signal();
        }
    };

    EXPECT_TRUE(sync_wait(exec, await_tag_task(event.handle, signaled)));
}

// same contract through the free function spelling, which is how callers actually use it
TAG_TEST(await_handle, resumes_after_signal) {
    co::scheduler sched{autostart, 1};
    co::executor exec{sched};

    scoped_event event{};
    std::atomic<bool> signaled{false};

    std::jthread const signaller{
        [&event, &signaled] {
            std::this_thread::sleep_for(std::chrono::milliseconds{25});
            signaled = true;
            event.signal();
        }
    };

    EXPECT_TRUE(sync_wait(exec, await_handle_task(event.handle, signaled)));
}

// the executor contract must survive the suspension: resumption is driven by the scheduler's completion
// handler, so the coroutine comes back on a worker -- never inline on the signalling thread or the caller.
TAG_TEST(await_handle, resumes_on_scheduler_thread) {
    co::scheduler sched{autostart, 1};
    co::executor exec{sched};

    scoped_event event{};

    auto task = [](os::native_handle_t h) -> executor_task<std::thread::id> {
        co_await this_coro::await_handle(h);
        co_return std::this_thread::get_id();
    };

    auto const caller = std::this_thread::get_id();
    std::atomic<std::thread::id> signaller{};

    std::jthread const signalThread{
        [&event, &signaller] {
            signaller = std::this_thread::get_id();
            std::this_thread::sleep_for(std::chrono::milliseconds{25});
            event.signal();
        }
    };

    auto const resumed = sync_wait(exec, task(event.handle));

    EXPECT_NE(resumed, caller);
    EXPECT_NE(resumed, signaller.load());
}

}
