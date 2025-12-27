#include  "cth/meta/coro.hpp"

#include "test.hpp"

#include <coroutine>

// Test types
struct with_operator_co_await {
    struct awaiter {
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<>) {}
        int await_resume() { return 42; }
    };

    awaiter operator co_await() { return {}; }
};

struct with_free_co_await {
    struct awaiter {
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<>) {}
        double await_resume() { return 3.14; }
    };
};

with_free_co_await::awaiter operator co_await(with_free_co_await) { return {}; }

struct not_awaitable {
    int value;
};

struct with_both_co_await {
    struct awaiter {
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<>) {}
        void* await_resume() { return nullptr; }
    };

    awaiter operator co_await() { return {}; }
};

with_both_co_await::awaiter operator co_await(with_both_co_await) { return {}; }

// Test 1: has_op_co_await concept
TYPE_TEST(has_op_co_await, main) {
    static_assert(cth::co::has_op_co_await<with_operator_co_await>);
    static_assert(!cth::co::has_op_co_await<with_free_co_await>);
    static_assert(!cth::co::has_op_co_await<not_awaitable>);
    static_assert(!cth::co::has_op_co_await<int>);
    static_assert(cth::co::has_op_co_await<with_both_co_await>);
}

// Test 2: has_free_co_await concept
TYPE_TEST(has_free_co_await, main) {
    static_assert(!cth::co::has_free_co_await<with_operator_co_await>);
    static_assert(cth::co::has_free_co_await<with_free_co_await>);
    static_assert(!cth::co::has_free_co_await<not_awaitable>);
    static_assert(!cth::co::has_free_co_await<int>);
    static_assert(cth::co::has_free_co_await<with_both_co_await>);
}

// Test 3: awaitable concept
TYPE_TEST(awaitable, main) {
    static_assert(cth::co::awaitable<with_operator_co_await>);
    static_assert(cth::co::awaitable<with_free_co_await>);
    static_assert(!cth::co::awaitable<not_awaitable>);
    static_assert(!cth::co::awaitable<int>);
    static_assert(!cth::co::awaitable<void>);
    static_assert(cth::co::awaitable<with_both_co_await>);
}

// Test 4: awaited_t type alias
TYPE_TEST(awaited_t, main) {
    static_assert(
        std::same_as<
            cth::co::awaiter_t<with_operator_co_await>,
            with_operator_co_await::awaiter
        >
    );

    static_assert(
        std::same_as<
            cth::co::awaiter_t<with_free_co_await>,
            with_free_co_await::awaiter
        >
    );

    static_assert(
        std::same_as<
            cth::co::awaiter_t<with_both_co_await>,
            with_both_co_await::awaiter
        >
    );
}
