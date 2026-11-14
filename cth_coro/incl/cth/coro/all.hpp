#pragma once
#include "cth/coro/awaiters/all_awaiter.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/coro.hpp>

#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>


namespace cth::co::dev {
/**
 * awaitables of `co::all()`
 * @details `co_await` it once, as an rvalue
 * @tparam Awaitables the awaitables, a reference -> keeps the reference
 */
template<class... Awaitables>
struct [[nodiscard]] all_group {
    std::tuple<Awaitables...> awaitables;

    /**
     * @return awaiter that owns the awaitables
     */
    [[nodiscard]] all_awaiter<Awaitables...> operator co_await() && {
        return all_awaiter<Awaitables...>{std::move(awaitables)};
    }
};

/**
 * range of awaitables of `co::all()`
 * @details `co_await` it once, as an rvalue
 * @tparam Range of awaitables, a reference -> keeps the reference
 */
template<class Range>
struct [[nodiscard]] all_range_group {
    Range range;

    /**
     * @return awaiter that owns the range
     */
    [[nodiscard]] all_range_awaiter<Range> operator co_await() && {
        return all_range_awaiter<Range>{std::forward<Range>(range)};
    }
};
}

namespace cth::co {
/**
 * `true` if `R` is a range for `co::all()`: a forward range of stored awaitables, not an awaitable itself
 */
template<class R>
concept all_range = std::ranges::forward_range<R> && !awaitable<R>
    && std::is_lvalue_reference_v<std::ranges::range_reference_t<R>>
    && awaitable<std::ranges::range_reference_t<R>>;

/**
 * awaits every awaitable
 * @details
 * - each awaitable runs in a child, the children start in turn on the calling thread until they suspend
 * - a ready awaiter gets no child: its `await_resume()` runs at once
 * - the children are `this_coro` coroutines with the context and the payload of the caller
 * - the caller resumes after the last child on its context
 * - every child ended at the start -> the caller does not suspend
 * - an lvalue stays a reference: it lives until the end of the `co_await`
 * - a child failed -> after the last child the `co_await` throws `cth::except::group_coro_exception`
 * @param awaitables to await
 * @return awaitable for a `this_coro` caller, gives a `std::tuple` of the results in argument order, no element for a
 *  void result, `void` if every result is void
 */
template<awaitable... Awaitables>
[[nodiscard]] auto all(Awaitables&&... awaitables) -> dev::all_group<Awaitables...> {
    return dev::all_group<Awaitables...>{std::tuple<Awaitables...>{std::forward<Awaitables>(awaitables)...}};
}

/**
 * awaits every awaitable of the range
 * @details
 * - the details of the overload with awaitables
 * - an lvalue range stays a reference: it lives until the end of the `co_await`
 * @param range of awaitables of one type, stored in the range
 * @return awaitable for a `this_coro` caller, gives a `std::vector` of the results in range order, a reference
 *  as a `cth::ref_wrapper`, `void` for void results
 */
template<all_range Range>
[[nodiscard]] auto all(Range&& range) -> dev::all_range_group<Range> {
    return dev::all_range_group<Range>{std::forward<Range>(range)};
}
}
