#include "test.hpp"

#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/this_coro.hpp"

#include <cth/meta/concepts.hpp>

#include <atomic>
#include <concepts>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace cth::co {

namespace {

    // a custom payload: a marker value, the built-in tags take the scheduler from the promise
    struct tagged_payload : this_coro::payload_base {
        constexpr explicit tagged_payload(int marker) : _marker{marker} {}

        [[nodiscard]] constexpr int marker() const { return _marker; }

    private:
        int _marker;
    };

    template<class P>
    concept marked_payload = requires(P const& p) {
        { p.marker() } -> std::convertible_to<int>;
    };

    // a custom this_coro tag that reads the marker off whatever payload the coroutine carries
    struct marker_tag : this_coro::tag_base {
        static constexpr auto operator()(co::scheduler const&, marked_payload auto const& p) {
            return data_awaiter{p.marker()};
        }
    };

    inline constexpr marker_tag marker{};

    struct parent_payload : this_coro::payload_base {};

    // the conversion from the payload of the parent throws
    struct throwing_payload : this_coro::payload_base {
        throwing_payload(parent_payload const&) { throw std::runtime_error("payload conversion"); }
    };

    static_assert(!mta::nothrow_constructible<throwing_payload, parent_payload const&>);

    // invariant guard: the awaiter of a this_coro task must derive this_coro_awaiter_base, else the caller
    // captures the task as a foreign awaitable and the task never gets the scheduler and the payload.
    // a compile error here beats a runtime deref of an empty payload.
    static_assert(this_coro_awaitable<executor_task<int>>, "executor_task lost its this_coro classification");
    static_assert(this_coro_awaitable<executor_task<int, tagged_payload>>);

} // namespace

THIS_CORO_TEST(custom_payload, marker_injected) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<int, tagged_payload> { co_return co_await marker; };

    auto const actual = exec.sync_wait<tagged_payload>(task(), 42);
    EXPECT_EQ(actual, 42);
}

THIS_CORO_TEST(custom_payload, propagates_through_nested) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto child = []() -> executor_task<int, tagged_payload> { co_return co_await marker; };
    auto root = [](auto c) -> executor_task<int, tagged_payload> { co_return co_await c(); };

    auto const actual = exec.sync_wait<tagged_payload>(root(child), 7);
    EXPECT_EQ(actual, 7);
}

THIS_CORO_TEST(custom_payload, deep_recursion_propagation) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    struct Recursive {
        auto operator()(int depth) -> executor_task<int, tagged_payload> {
            if(depth == 0)
                co_return co_await marker;
            co_return co_await (*this)(depth - 1);
        }
    };

    auto const actual = exec.sync_wait<tagged_payload>(Recursive{}(8), 99);
    EXPECT_EQ(actual, 99);
}

THIS_CORO_TEST(custom_payload, builtin_executor_tag_coexists) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    // the built-in tags take the scheduler from the promise, they work with every payload
    auto task = []() -> executor_task<executor, tagged_payload> { co_return co_await this_coro::executor; };

    auto const actual = exec.sync_wait<tagged_payload>(task(), 1);
    EXPECT_EQ(actual, exec);
}

THIS_CORO_TEST(custom_payload, custom_and_builtin_tags_in_one_task) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<int, tagged_payload> {
        (void) co_await this_coro::executor; // built-in tag resolves under the custom payload
        co_return co_await marker;          // custom tag reads the marker of the payload
    };

    auto const actual = exec.sync_wait<tagged_payload>(task(), 5);
    EXPECT_EQ(actual, 5);
}

// the custom payload slices to the default payload of the child
THIS_CORO_TEST(custom_payload, awaits_default_payload_task) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto child = []() -> executor_task<executor> { co_return co_await this_coro::executor; };
    auto root = [](auto c) -> executor_task<std::pair<executor, int>, tagged_payload> {
        auto const e = co_await c();
        co_return std::pair{e, co_await marker};
    };

    auto const [actual, m] = exec.sync_wait<tagged_payload>(root(child), 4);
    EXPECT_EQ(actual, exec);
    EXPECT_EQ(m, 4);
}

// a throwing payload copy reaches the caller, the child does not start
THIS_CORO_TEST(custom_payload, throwing_copy_reaches_caller) {
    scheduler sched{autostart, 1};
    executor exec{sched};
    std::atomic<bool> childStarted{false};

    auto child = [](std::atomic<bool>& started) -> executor_task<void, throwing_payload> {
        started = true;
        co_return;
    };
    auto root = [](scheduler const& s, std::atomic<bool>& started, auto c)
        -> executor_task<bool, parent_payload> {
        bool caughtOnSched = false;
        try {
            co_await c(started);
        } catch(std::runtime_error const&) {
            caughtOnSched = s.owns_thread();
        }
        co_return caughtOnSched;
    };

    EXPECT_TRUE(exec.sync_wait<parent_payload>(root(sched, childStarted, child)));
    EXPECT_FALSE(childStarted);
}

// schedule() changes the scheduler, the payload stays
THIS_CORO_TEST(custom_payload, schedule_keeps_payload) {
    scheduler home{autostart, 1};
    scheduler other{autostart, 1};
    executor homeExec{home};

    auto task = [](executor target) -> executor_task<std::tuple<int, scheduler const*, bool>, tagged_payload> {
        co_await target.schedule();
        int const m = co_await marker;
        scheduler const& current = co_await this_coro::scheduler;
        co_return std::tuple{m, &current, target.scheduler().owns_thread()};
    };

    auto const [m, current, onOther] = homeExec.sync_wait<tagged_payload>(task(executor{other}), 3);
    EXPECT_EQ(m, 3);
    EXPECT_EQ(current, &other);
    EXPECT_TRUE(onOther);
}

}
