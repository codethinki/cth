#pragma once
#include "cth/coro/awaiters/raw_native_handle_awaiter.hpp"
#include "cth/coro/utility/fwd.hpp"

namespace cth::co::this_coro {
/**
 * tag of `await_handle()`
 */
struct [[nodiscard]] native_handle_tag : tag_base {
    constexpr explicit native_handle_tag(os::native_handle_t h) : handle{h} {}
    /**
     * waits on the handle on the context of the caller
     * @return awaiter that waits on the handle
     */
    constexpr auto operator()(co::context const& c, auto const&) const {
        return raw_native_handle_awaiter{c, handle};
    }
    os::native_handle_t handle;
};

/**
 * waits until the handle is signaled, no wait if it is signaled
 * @param h to wait on
 * @return tag, `co_await` it
 */
constexpr auto await_handle(os::native_handle_t h) { return native_handle_tag{h}; }
}
