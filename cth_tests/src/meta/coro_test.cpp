#include "cth/meta/coro.hpp"

#include "test.hpp"

#include <coroutine>

namespace cth::co {

template<class T>
struct basic_awaiter {
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<>) {}
    T await_resume() { return {}; }
};

template<class T>
struct basic_with_operator_co_await {
    using awaiter = basic_awaiter<T>;

    awaiter operator co_await() { return {}; }
};

using with_operator_co_await = basic_with_operator_co_await<int>;

struct with_free_co_await {
    using awaiter = basic_awaiter<double>;
};

with_free_co_await::awaiter operator co_await(with_free_co_await) { return {}; }

struct not_awaitable {
    int value;
};

struct with_both_co_await {
    using awaiter = basic_awaiter<void*>;

    awaiter operator co_await() { return {}; }
};


with_both_co_await::awaiter operator co_await(with_both_co_await) { return {}; }


TYPE_TEST(has_op_co_await, main) {
    static_assert(has_op_co_await<with_operator_co_await>);
    static_assert(!has_op_co_await<with_free_co_await>);
    static_assert(!has_op_co_await<not_awaitable>);
    static_assert(!has_op_co_await<int>);
    static_assert(has_op_co_await<with_both_co_await>);
}

TYPE_TEST(has_free_co_await, main) {
    static_assert(!has_free_co_await<with_operator_co_await>);
    static_assert(has_free_co_await<with_free_co_await>);
    static_assert(!has_free_co_await<not_awaitable>);
    static_assert(!has_free_co_await<int>);
    static_assert(has_free_co_await<with_both_co_await>);
}

TYPE_TEST(awaitable, main) {
    static_assert(awaitable<with_operator_co_await>);
    static_assert(awaitable<with_free_co_await>);
    static_assert(!awaitable<not_awaitable>);
    static_assert(!awaitable<int>);
    static_assert(!awaitable<void>);
    static_assert(awaitable<with_both_co_await>);
}

TYPE_TEST(awaiter_t, main) {
    static_assert(std::same_as<awaiter_t<basic_with_operator_co_await<int&>>, basic_awaiter<int&>>);

    static_assert(std::same_as<awaiter_t<with_operator_co_await>, with_operator_co_await::awaiter>);

    static_assert(std::same_as<awaiter_t<with_free_co_await>, with_free_co_await::awaiter>);

    static_assert(std::same_as<awaiter_t<with_both_co_await>, with_both_co_await::awaiter>);
}


TYPE_TEST(awaited_t, main) {
    static_assert(std::same_as<awaited_t<basic_awaiter<double>>, double>);

    static_assert(std::same_as<awaited_t<basic_awaiter<double&>>, double&>);

    static_assert(std::same_as<awaited_t<basic_with_operator_co_await<double&>>, double&>);
}

}
