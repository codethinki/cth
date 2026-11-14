#include "test.hpp"

#include "cth/coro/all.hpp"
#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/exception.hpp"
#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/this_coro.hpp"

#include <atomic>
#include <chrono>
#include <concepts>
#include <exception>
#include <ranges>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>


namespace cth::co {
namespace {
    [[nodiscard]] executor_task<int> value(int v) { co_return v; }

    [[nodiscard]] executor_task<int&> reference(int& v) { co_return v; }

    [[nodiscard]] executor_task<void> count(std::atomic<int>& ran) {
        ++ran;
        co_return;
    }

    // suspends before it counts
    [[nodiscard]] executor_task<void> count_later(std::atomic<int>& ran) {
        co_await this_coro::wait(std::chrono::milliseconds{2});
        ++ran;
    }

    [[nodiscard]] executor_task<int> fail(int code) {
        throw std::runtime_error{std::format("fail {}", code)};
        co_return 0;
    }

    [[nodiscard]] executor_task<void> fail_without_std_exception() {
        throw 42;
        co_return;
    }

    // the type of co_await co::all() on the arguments
    template<class... Awaitables>
    using all_t = awaited_t<decltype(co::all(std::declval<Awaitables>()...))>;

    static_assert(
        std::same_as<all_t<executor_task<int>, executor_task<void>, executor_task<int&>>, std::tuple<int, int&>>
    );
    static_assert(std::is_void_v<all_t<executor_task<void>, executor_task<void>>>);
    static_assert(std::same_as<all_t<std::vector<executor_task<int>>&>, std::vector<int>>);
    static_assert(std::is_void_v<all_t<std::vector<executor_task<void>>&>>);
    static_assert(this_coro_awaitable<decltype(co::all(std::declval<executor_task<int>>()))>);
}

CORO_TEST(all, tuple_of_the_results_without_the_void_children) {
    scheduler sched{autostart, 2};
    executor exec{sched};
    std::atomic<int> ran{0};
    int target = 7;

    auto root = [](std::atomic<int>& r, int& t) -> executor_task<std::tuple<int, int*, int>> {
        auto lvalue = value(3);
        auto [first, ref, last] = co_await co::all(value(1), count(r), reference(t), count_later(r), lvalue);
        co_return std::tuple{first, &ref, last};
    };

    auto const [first, refPtr, last] = exec.sync_wait(root(ran, target));
    EXPECT_EQ(first, 1);
    EXPECT_EQ(refPtr, &target);
    EXPECT_EQ(last, 3);
    EXPECT_EQ(ran.load(), 2);
}

CORO_TEST(all, void_children_give_void) {
    scheduler sched{autostart, 2};
    executor exec{sched};
    std::atomic<int> ran{0};

    auto root = [](std::atomic<int>& r) -> executor_task<void> {
        co_await co::all(count(r), count_later(r), count(r));
    };

    exec.sync_wait(root(ran));
    EXPECT_EQ(ran.load(), 3);
}

CORO_TEST(all, range_gives_a_vector_of_the_results) {
    scheduler sched{autostart, 2};
    executor exec{sched};

    auto root = []() -> executor_task<std::vector<int>> {
        std::vector<executor_task<int>> tasks{};
        for(int i = 0; i < 5; ++i)
            tasks.push_back(value(i));
        co_return co_await co::all(tasks);
    };

    EXPECT_EQ(exec.sync_wait(root()), (std::vector{0, 1, 2, 3, 4}));
}

CORO_TEST(all, range_of_void_children) {
    scheduler sched{autostart, 2};
    executor exec{sched};
    std::atomic<int> ran{0};

    auto root = [](std::atomic<int>& r) -> executor_task<void> {
        std::vector<executor_task<void>> tasks{};
        for(int i = 0; i < 4; ++i)
            tasks.push_back(i % 2 == 0 ? count(r) : count_later(r));
        co_await co::all(std::move(tasks));
    };

    exec.sync_wait(root(ran));
    EXPECT_EQ(ran.load(), 4);
}

CORO_TEST(all, transform_view_of_tasks) {
    scheduler sched{autostart, 2};
    executor exec{sched};
    std::atomic<int> ran{0};

    // the view makes the tasks: each child owns its task
    auto root = [](std::atomic<int>& r) -> executor_task<std::vector<int>> {
        std::vector const inputs{1, 2, 3};
        auto results = co_await co::all(inputs | std::views::transform(value));
        co_await co::all(std::views::iota(0, 4) | std::views::transform([&r](int) { return count_later(r); }));
        co_return results;
    };

    EXPECT_EQ(exec.sync_wait(root(ran)), (std::vector{1, 2, 3}));
    EXPECT_EQ(ran.load(), 4);
}

CORO_TEST(all, transform_view_of_awaiters) {
    scheduler home{autostart, 1};
    scheduler other{autostart, 1};
    executor exec{home};

    auto root = [](scheduler const& h, scheduler const& o)
        -> executor_task<std::tuple<std::vector<int>, context const*, bool>> {
        std::vector const inputs{4, 5};
        // ready: the results at once
        auto ready = co_await co::all(inputs | std::views::transform([](int i) { return data_awaiter{i}; }));
        // not ready: each child owns its awaiter, it moves to the other context
        co_await co::all(inputs | std::views::transform([&o](int) { return executor{o}.schedule(); }));
        context const& current = co_await this_coro::context;
        co_return std::tuple{ready, &current, h.owns_thread()};
    };

    auto const [ready, current, onHome] = exec.sync_wait(root(home, other));
    EXPECT_EQ(ready, (std::vector{4, 5}));
    EXPECT_EQ(current, &home);
    EXPECT_TRUE(onHome);
}

CORO_TEST(all, empty_range_gives_an_empty_vector) {
    scheduler sched{autostart, 1};
    executor exec{sched};

    auto root = []() -> executor_task<std::vector<int>> {
        std::vector<executor_task<int>> const tasks{};
        co_return co_await co::all(tasks);
    };

    EXPECT_TRUE(exec.sync_wait(root()).empty());
}

CORO_TEST(all, ready_awaiters_and_inline_children_do_not_suspend) {
    scheduler sched{autostart, 1};
    executor exec{sched};
    std::atomic<bool> otherJob{false};

    // one worker: a suspension runs the job posted before it first
    auto root = [](scheduler const& s, std::atomic<bool>& other)
        -> executor_task<std::tuple<bool, int, std::string, int>> {
        s.post([&other] { other = true; });
        auto [number, text] = co_await co::all(data_awaiter{1}, data_awaiter{std::string{"text"}});
        auto [inlineValue] = co_await co::all(value(2));
        co_return std::tuple{other.load(), number, text, inlineValue};
    };

    auto const [suspended, number, text, inlineValue] = exec.sync_wait(root(sched, otherJob));
    EXPECT_FALSE(suspended);
    EXPECT_EQ(number, 1);
    EXPECT_EQ(text, "text");
    EXPECT_EQ(inlineValue, 2);
}

CORO_TEST(all, children_move_to_another_context_the_caller_resumes_on_its_own) {
    scheduler home{autostart, 1};
    scheduler other{autostart, 1};
    executor exec{home};

    auto moved = [](scheduler const& target) -> executor_task<bool> {
        co_await executor{target}.schedule();
        co_return target.owns_thread();
    };

    auto root = [](scheduler const& h, scheduler const& o, auto m)
        -> executor_task<std::tuple<bool, bool, context const*, bool>> {
        // the child of the schedule awaiter itself ends on the other context
        auto [first, second] = co_await co::all(m(o), executor{o}.schedule(), m(o));
        context const& current = co_await this_coro::context;
        co_return std::tuple{first, second, &current, h.owns_thread()};
    };

    auto const [first, second, current, onHome] = exec.sync_wait(root(home, other, moved));
    EXPECT_TRUE(first);
    EXPECT_TRUE(second);
    EXPECT_EQ(current, &home);
    EXPECT_TRUE(onHome);
}

CORO_TEST(all, one_failed_child_throws_after_every_child_ended) {
    scheduler sched{autostart, 2};
    executor exec{sched};
    std::atomic<int> ran{0};

    struct failure {
        std::vector<bool> failed;
        std::string what;
        int ranAtCatch;
    };

    auto root = [](std::atomic<int>& r) -> executor_task<failure> {
        try {
            co_await co::all(count_later(r), fail(1), count_later(r));
        }
        catch(except::group_coro_exception const& e) {
            std::vector<bool> failed{};
            for(auto const& exception : e.exceptions())
                failed.push_back(exception != nullptr);
            co_return failure{failed, e.what(), r.load()};
        }
        co_return failure{};
    };

    auto const [failed, what, ranAtCatch] = exec.sync_wait(root(ran));
    EXPECT_EQ(failed, (std::vector{false, true, false}));
    EXPECT_NE(what.find("1 of 3 children of co::all() failed"), std::string::npos);
    EXPECT_NE(what.find("child 1: fail 1"), std::string::npos);
    EXPECT_EQ(ranAtCatch, 2);
}

CORO_TEST(all, several_failed_children) {
    scheduler sched{autostart, 2};
    executor exec{sched};
    std::atomic<int> ran{0};

    auto root = [](std::atomic<int>& r) -> executor_task<std::pair<std::vector<std::exception_ptr>, std::string>> {
        try {
            co_await co::all(fail(0), count_later(r), fail_without_std_exception(), fail(3), value(4));
        }
        catch(except::group_coro_exception const& e) {
            co_return std::pair{e.exceptions(), std::string{e.what()}};
        }
        co_return std::pair<std::vector<std::exception_ptr>, std::string>{};
    };

    auto const [exceptions, what] = exec.sync_wait(root(ran));
    ASSERT_EQ(exceptions.size(), 5);
    EXPECT_NE(exceptions[0], nullptr);
    EXPECT_EQ(exceptions[1], nullptr);
    EXPECT_NE(exceptions[2], nullptr);
    EXPECT_NE(exceptions[3], nullptr);
    EXPECT_EQ(exceptions[4], nullptr);
    EXPECT_THROW(std::rethrow_exception(exceptions[3]), std::runtime_error);
    EXPECT_NE(what.find("3 of 5 children of co::all() failed"), std::string::npos);
    EXPECT_NE(what.find("child 0: fail 0"), std::string::npos);
    EXPECT_NE(what.find("child 2: an exception that does not derive std::exception"), std::string::npos);
    EXPECT_NE(what.find("child 3: fail 3"), std::string::npos);
    EXPECT_EQ(ran.load(), 1);
}

CORO_TEST(all, failed_child_of_a_range) {
    scheduler sched{autostart, 2};
    executor exec{sched};

    auto root = []() -> executor_task<std::vector<bool>> {
        std::vector<executor_task<int>> tasks{};
        tasks.push_back(value(0));
        tasks.push_back(fail(1));
        tasks.push_back(value(2));
        try {
            co_await co::all(tasks);
        }
        catch(except::group_coro_exception const& e) {
            std::vector<bool> failed{};
            for(auto const& exception : e.exceptions())
                failed.push_back(exception != nullptr);
            co_return failed;
        }
        co_return std::vector<bool>{};
    };

    EXPECT_EQ(exec.sync_wait(root()), (std::vector{false, true, false}));
}
}
