#pragma once
#include "cth/coro/this_coro/payload.hpp"
#include "cth/coro/this_coro/this_coro_base.hpp"

#include <cth/data/optional.hpp>

#include <coroutine>

namespace cth::co::dev {
/**
 * Base for awaitables which support payload injection
 * @tparam Pyld payload to work with defaulted to @ref this_coro::default_payload
 */
template<payload Pyld>
struct this_coro_awaiter_base : this_coro_base<Pyld> {
    constexpr this_coro_awaiter_base() = default;

    /**
     * Injects the payload into the promise
     *
     * @tparam Promise type, must be @ref executor_injectable and compatible with @ref Pyld
     * @param h handle with promise to inject executor into
     */
    template<promise Promise>
    void inject_payload_into(std::coroutine_handle<Promise> h) {
        constexpr bool injectable = this_coro::promise_injectable_with<Promise, Pyld>;

        static_assert(
            injectable,
            "this coro supporting awaitables may not be awaited by awaitables without a compatible payload"
        );

        if constexpr(injectable)
            h.promise().inject(*_payload);
    }

    /**
     * injects the this_coro payload into this awaiter unless it already has one
     * @param payload to inject
     */
    constexpr void inject(Pyld const& payload) {
        if(!_payload)
            _payload.emplace(payload);
    }

protected:
    Pyld& payload() { return *_payload; }

private:
    cth::dt::optional<Pyld> _payload{};
};
}
