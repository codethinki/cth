#pragma once
#include "cth/coro/awaiters/all_awaiter.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/coro.hpp>

#include <ranges>
#include <tuple>
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
 * `true` if `R` is a range for `co::all()`: a forward range of awaitables, not an awaitable itself
 */
template<class R>
concept all_range = std::ranges::forward_range<R> && !awaitable<R> && awaitable<std::ranges::range_reference_t<R>>;

/**
 * awaits every awaitable at once
 * @details
 * - the children start in turn on the calling thread, the caller resumes after the last one on its context
 * - a child failed -> throws `cth::except::group_coro_exception` after the last child
 * - a ready awaiter gives its result at once, no coroutine
 * - allocates a coroutine per child that is no ready awaiter, nothing else
 * @param awaitables to await, an lvalue stays a reference until the end of the `co_await`
 * @return awaitable, gives a `std::tuple` of the results, no element for a void result, `void` if all are void
 */
template<awaitable... Awaitables>
[[nodiscard]] auto all(Awaitables&&... awaitables) -> dev::all_group<Awaitables...> {
    return dev::all_group<Awaitables...>{std::tuple<Awaitables...>{std::forward<Awaitables>(awaitables)...}};
}

/**
 * awaits every awaitable of the range at once
 * @details the details of `all()` with awaitables
 * @details the allocations:
 * - a coroutine per child that is no ready awaiter
 * - `void` result: one vector of the children with a coroutine or a failure, none without such a child
 * - other result: one vector of every child, then the vector of the results
 * @param range of awaitables, an lvalue stays a reference until the end of the `co_await`
 * @return awaitable, gives a `std::vector` of the results, a reference as `cth::ref_wrapper`, `void` for void results
 */
template<all_range Range>
[[nodiscard]] auto all(Range&& range) -> dev::all_range_group<Range> {
    return dev::all_range_group<Range>{std::forward<Range>(range)};
}
}
