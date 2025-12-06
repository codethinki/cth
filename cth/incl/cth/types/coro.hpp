#pragma once
#include <type_traits>
#include <coroutine>
#include <concepts>
#include <utility>

namespace cth::co {
template<class T>
concept awaiter = requires(T t, std::coroutine_handle<> h) {
    { t.await_ready() } -> std::convertible_to<bool>;
    { t.await_suspend(h) };
    { t.await_resume() };
};

template<class T>
concept has_op_co_await = requires() {
    { std::declval<T>().operator co_await() } -> awaiter;
};

template<class T>
concept has_free_co_await = requires() {
    { operator co_await(std::declval<T>()) } -> awaiter;
};

template<class T>
concept task = has_op_co_await<T> || has_free_co_await<T>;

template<class T>
concept awaitable = task<T> || awaiter<T>;

}

namespace cth::co::dev {
template<awaitable Awaitable>
consteval auto awaiter_type() {
    if constexpr(!task<Awaitable>) { //!task<T> intentional, bc task<T> && awaiter<T> is possible
        static_assert(awaiter<Awaitable>, "must be an awaiter");
        return std::type_identity<Awaitable>{};
    } else if constexpr(has_op_co_await<Awaitable>) {
        using R = decltype(std::declval<Awaitable>().operator co_await());
        return std::type_identity<R>{};
    } else if constexpr(has_free_co_await<Awaitable>) {
        using R = decltype(operator co_await(std::declval<Awaitable>()));
        return std::type_identity<R>{};
    } else
        static_assert(false, "type is not an awaiter nor task");
}

}

namespace cth::co {
template<class Awaitable>
using awaiter_t = typename decltype(dev::awaiter_type<Awaitable>())::type;
}

namespace cth::co::dev {
template<awaitable Awaitable>
consteval auto awaited_type() {
    using Awaiter = awaiter_t<Awaitable>;

    using R = decltype(std::declval<Awaiter>().await_resume());
    return std::type_identity<R>{};
}
}

namespace cth::co {
template<class Awaitable>
using awaited_t = typename decltype(dev::awaited_type<Awaitable>())::type;

template<class Awaitable>
concept void_awaitable = awaitable<Awaitable> && std::same_as<void, type::rcvr_t<awaited_t<Awaitable>>>;

}
