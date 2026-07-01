#include "test.hpp"

#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/this_coro.hpp"

#include <concepts>

namespace cth::co {

namespace {

    // a custom payload: the scheduler context (so the built-in tags keep working) plus a marker value
    struct tagged_payload : this_coro::scheduler_payload_base {
        constexpr tagged_payload(co::scheduler const& s, int marker)
            : this_coro::scheduler_payload_base{s}, _marker{marker} {}

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
        static constexpr auto operator()(marked_payload auto const& p) { return data_awaiter{p.marker()}; }
    };

    inline constexpr marker_tag marker{};

    // invariant guard for the exact regression that caused the empty-payload bug: the awaiter must
    // expose payload_type, else this_coro tasks silently demote to the foreign/steal path and their
    // payload optional is never injected. a compile error here beats a runtime deref of an empty optional.
    static_assert(this_coro_awaitable<executor_task<int>>, "executor_task lost its this_coro classification");
    static_assert(this_coro_awaitable<executor_task<int, tagged_payload>>);

} // namespace

THIS_CORO_TEST(custom_payload, marker_injected) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<int, tagged_payload> { co_return co_await marker; };

    auto const actual = sync(exec.spawn<tagged_payload>(task(), 42));
    EXPECT_EQ(actual, 42);
}

THIS_CORO_TEST(custom_payload, propagates_through_nested) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto child = []() -> executor_task<int, tagged_payload> { co_return co_await marker; };
    auto root = [](auto c) -> executor_task<int, tagged_payload> { co_return co_await c(); };

    auto const actual = sync(exec.spawn<tagged_payload>(root(child), 7));
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

    auto const actual = sync(exec.spawn<tagged_payload>(Recursive{}(8), 99));
    EXPECT_EQ(actual, 99);
}

THIS_CORO_TEST(custom_payload, builtin_executor_tag_coexists) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    // tagged_payload derives scheduler_payload_base, so the built-in tags still resolve against it
    auto task = []() -> executor_task<executor, tagged_payload> { co_return co_await this_coro::executor; };

    auto const actual = sync(exec.spawn<tagged_payload>(task(), 1));
    EXPECT_EQ(actual, exec);
}

THIS_CORO_TEST(custom_payload, custom_and_builtin_tags_in_one_task) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto task = []() -> executor_task<int, tagged_payload> {
        (void) co_await this_coro::executor; // built-in tag resolves under the custom payload
        co_return co_await marker;          // custom tag reads the injected marker
    };

    auto const actual = sync(exec.spawn<tagged_payload>(task(), 5));
    EXPECT_EQ(actual, 5);
}

}
