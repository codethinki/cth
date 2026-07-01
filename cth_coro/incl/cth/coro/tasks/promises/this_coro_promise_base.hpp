#pragma once
#include "cth/coro/utility.hpp"
#include "cth/coro/func/steal.hpp"
#include "cth/coro/tasks/dev/capture_task.hpp"
#include "cth/coro/this_coro/payload.hpp"
#include "cth/coro/utility/concepts.hpp"

#include <cth/data/optional.hpp>
#include <cth/meta/coro.hpp>

#include <utility>


namespace cth::co {
class executor;


/**
 * promises supporting the libs this_coro namespace must publicly inherit from this
 * @details
 * - children must forward the payload on construction
 */
template<payload Pyld>
struct this_coro_promise_base : this_coro_base<Pyld> {
    template<this_coro::tag Tag>
    [[nodiscard]] constexpr decltype(auto) await_transform(Tag&& t) {
        static_assert(
            requires(Tag t) { { t.operator()(std::declval<Pyld>()) } -> awaitable; },
            "Pyld must satisfy the tags call operator"
        );

        return std::forward<Tag>(t).operator()(*_payload);
    }


    template<this_coro_awaitable Awaitable>
    [[nodiscard]] constexpr auto await_transform(Awaitable&& awaitable) -> awaiter_t<Awaitable> {
        static_assert(
            this_coro::awaitable_injectable_with<Awaitable, Pyld>,
            "awaited task carries a payload incompatible with this coroutine's payload"
        );

        auto awaiter = co::extract_awaiter(awaitable);

        awaiter.inject(*_payload);

        return awaiter;
    }

    template<captured_awaitable Awaitable>
    [[nodiscard]] constexpr auto await_transform(Awaitable&& captured) { return cth::co::extract_awaiter(captured); }

    template<foreign_awaitable Awaitable> requires (this_coro::scheduler_payload<Pyld>)
    [[nodiscard]] auto await_transform(Awaitable awaitable) -> capture_task<awaited_t<Awaitable>> {
        return co::steal(std::move(awaitable), *_payload);
    }

    /**
     * injects the this_coro default_payload into the promise
     * @param payload to inject
     */
    constexpr void inject(Pyld const& payload) { _payload = payload; }

    /**
     * repoints the promise's payload at a new scheduler, preserving every other payload field
     * @details only available for scheduler payloads; the payload is engaged on every path that reaches
     *  this (a this_coro co_await goes through await_transform, which derefs the payload first)
     * @param scheduler the coroutine has migrated onto
     */
    constexpr void rebind_scheduler(co::scheduler const& scheduler)
        requires this_coro::scheduler_payload<Pyld>
    {
        _payload->rebind_scheduler(scheduler);
    }

    this_coro_promise_base(Pyld const& payload) noexcept : _payload{payload} {}
    this_coro_promise_base() noexcept = default;

private:
    cth::dt::optional<Pyld> _payload{};
};

}
