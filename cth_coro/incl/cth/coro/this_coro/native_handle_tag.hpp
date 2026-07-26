#pragma once
#include "cth/coro/awaiters/native_handle_awaiter.hpp"
#include "cth/coro/this_coro/payload.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <chrono>


namespace cth::co::this_coro {
struct [[nodiscard]] native_handle_tag : tag_base {
    constexpr explicit native_handle_tag(os::native_handle_t h) : handle{h} {}
    constexpr auto operator()(scheduler_payload auto const& p) const {
        return native_handle_awaiter{p.scheduler(), handle};
    }
    os::native_handle_t handle;
};

/**
 * waits until the native handle is signaled, noop if it already is
 * @param h to wait on
 * @return native handle tag
 */
constexpr auto await_handle(os::native_handle_t h) { return native_handle_tag{h}; }
}
