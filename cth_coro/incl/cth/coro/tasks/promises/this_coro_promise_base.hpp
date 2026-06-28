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
struct this_coro_promise_base {
    using payload_type = Pyld;

    template<this_coro::tag Tag>
    [[nodiscard]] constexpr decltype(auto) await_transform(Tag&& t) {
        static_assert(
            requires(Tag t) { { t.operator()(std::declval<Pyld>()) } -> awaitable; },
            "Pyld must satisfy the tags call operator"
        );

        return std::forward<Tag>(t).operator()(*_payload);
    }


    template<this_coro_awaitable_with<Pyld> Awaitable>
    [[nodiscard]] constexpr auto await_transform(Awaitable&& awaitable) -> awaiter_t<Awaitable> {
        auto awaiter = co::extract_awaiter(awaitable);

        awaiter.inject(*_payload);

        return awaiter;
    }

    // a this_coro awaitable whose payload Pyld cannot satisfy -- diagnose instead of falling through to
    // "no viable await_transform" (it is not foreign: foreign excludes this_coro awaitables by design)
    template<class Awaitable>
        requires (this_coro_awaitable<Awaitable> && !this_coro_awaitable_with<Awaitable, Pyld>)
    [[nodiscard]] constexpr auto await_transform(Awaitable&& awaitable) -> awaiter_t<Awaitable> {
        static_assert(
            this_coro_awaitable_with<Awaitable, Pyld>,
            "awaited task carries a payload incompatible with this coroutine's payload"
        );
        return co::extract_awaiter(std::forward<Awaitable>(awaitable));
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

    this_coro_promise_base(Pyld const& payload) noexcept : _payload{payload} {}
    this_coro_promise_base() noexcept = default;

private:
    cth::dt::optional<Pyld> _payload{};
};

}
