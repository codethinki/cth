#pragma once
#include "cth/coro/this_coro/payload.hpp"

#include "cth/coro/utility/concepts.hpp"
#include <coroutine>
#include <optional>


namespace cth::co {
/**
 * Base for awaitables which support executor injection
 */
struct this_coro_awaiter_base {
    this_coro_awaiter_base() = default;

    /**
     * Injects the executor into the current promise
     *
     * @tparam Promise type, must be @ref executor_injectable
     * @param h handle with promise to inject executor into
     */
    template<promise Promise>
    void inject_payload_into(std::coroutine_handle<Promise> h) {
        constexpr bool injectable = this_coro_supported_promise<Promise>;

        static_assert(
            injectable,
            "this coro supporting awaitables may not be awaited by awaitables without support"
        );


        if constexpr(injectable) h.promise().inject(*_payload);
    }

    /**
     * injects the this_coro payload into this awaiter unless it already has one
     * @param payload to inject
     */
    void inject(this_coro::payload payload) { if(!_payload) _payload.emplace(payload); }

private:
    std::optional<this_coro::payload> _payload{};
};



}
