#pragma once
#include "cth/coro/awaiters/all_awaiter.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/coro.hpp>

#include <execution>
#include <ranges>
#include <tuple>
#include <utility>


namespace cth::co::dev {
/**
 * awaitables of `co::all()`
 * @details `co_await` it once, as an rvalue
 * @tparam Policy how the children start, see `co::all()`
 * @tparam Awaitables the awaitables, a reference -> keeps the reference
 */
template<all_policy Policy, class... Awaitables>
struct [[nodiscard]] all_group {
    std::tuple<Awaitables...> awaitables;

    /**
     * @return awaiter that owns the awaitables
     */
    [[nodiscard]] all_awaiter<Policy, Awaitables...> operator co_await() && {
        return all_awaiter<Policy, Awaitables...>{std::move(awaitables)};
    }
};

/**
 * range of awaitables of `co::all()`
 * @details `co_await` it once, as an rvalue
 * @tparam Policy how the children start, see `co::all()`
 * @tparam Range of awaitables, a reference -> keeps the reference
 */
template<all_policy Policy, class Range>
struct [[nodiscard]] all_range_group {
    Range range;

    /**
     * @return awaiter that owns the range
     */
    [[nodiscard]] all_range_awaiter<Policy, Range> operator co_await() && {
        return all_range_awaiter<Policy, Range>{std::forward<Range>(range)};
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
 * @details `std::execution::seq`: the children start in turn on the calling thread, each runs until it
 *  suspends
 * @details `std::execution::par`:
 * - every child but the last is posted to the context of the caller, they run at the same time
 * - the last child starts inline on the calling thread
 * - a strand as the context runs the posted children one at a time, after the job of the caller
 * - a failed `context::post()`: the child starts inline
 * @details both policies:
 * - the caller resumes after the last child on its context
 * - a child failed -> throws `cth::except::group_coro_exception` after the last child
 * - a ready awaiter gives its result at once, no coroutine
 * - allocates a coroutine per child that is no ready awaiter, `std::execution::par` also the posts
 * @tparam Policy `std::execution::sequenced_policy` or `std::execution::parallel_policy`
 * @param awaitables to await, an lvalue stays a reference until the end of the `co_await`
 * @return awaitable, gives a `std::tuple` of the results, no element for a void result, `void` if all are void
 * @see `all_policy`
 */
template<all_policy Policy, awaitable... Awaitables>
[[nodiscard]] auto all(Policy, Awaitables&&... awaitables) -> dev::all_group<Policy, Awaitables...> {
    return dev::all_group<Policy, Awaitables...>{
        std::tuple<Awaitables...>{std::forward<Awaitables>(awaitables)...}
    };
}

/**
 * awaits every awaitable at once, `std::execution::par`
 * @details no policy -> `std::execution::par`
 * @details the details of @ref all(Policy, Awaitables&&...)
 */
template<awaitable... Awaitables>
[[nodiscard]] auto all(Awaitables&&... awaitables)
    -> dev::all_group<std::execution::parallel_policy, Awaitables...> {
    return co::all(std::execution::par, std::forward<Awaitables>(awaitables)...);
}

/**
 * awaits every awaitable of the range at once
 * @details the details of `all()` with awaitables, the policies included
 * @details the allocations:
 * - a coroutine per child that is no ready awaiter
 * - `void` result: one vector of the children with a coroutine or a failure, none without such a child
 * - other result: one vector of every child, then the vector of the results
 * - `std::execution::par`: the posts
 * @tparam Policy `std::execution::sequenced_policy` or `std::execution::parallel_policy`
 * @param range of awaitables, an lvalue stays a reference until the end of the `co_await`
 * @return awaitable, gives a `std::vector` of the results, a reference as `cth::ref_wrapper`, `void` for void results
 */
template<all_policy Policy, all_range Range>
[[nodiscard]] auto all(Policy, Range&& range) -> dev::all_range_group<Policy, Range> {
    return dev::all_range_group<Policy, Range>{std::forward<Range>(range)};
}

/**
 * awaits every awaitable of the range at once, `std::execution::par`
 * @details no policy -> `std::execution::par`
 * @details the details of @ref all(Policy, Range&&)
 */
template<all_range Range>
[[nodiscard]] auto all(Range&& range) -> dev::all_range_group<std::execution::parallel_policy, Range> {
    return co::all(std::execution::par, std::forward<Range>(range));
}
}
