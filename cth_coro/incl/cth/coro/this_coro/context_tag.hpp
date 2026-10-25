#pragma once
#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/utility/fwd.hpp"


namespace cth::co::this_coro {
struct [[nodiscard]] context_tag : tag_base {
    static constexpr auto operator()(co::context const& c, auto const&) { return data_awaiter<co::context const&>{c}; }
};

inline constexpr context_tag context{};
}
