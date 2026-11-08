#pragma once
#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/utility/fwd.hpp"


namespace cth::co::this_coro {
/**
 * tag of `this_coro::context`
 */
struct [[nodiscard]] context_tag : tag_base {
    /**
     * gives the context of the caller
     * @return ready awaiter with the context
     */
    static constexpr auto operator()(co::context const& c, auto const&) { return data_awaiter<co::context const&>{c}; }
};

/**
 * `co_await` it: gives the context of the caller
 */
inline constexpr context_tag context{};
}
