#pragma once
#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/utility/fwd.hpp"
#include "payload.hpp"


namespace cth::co::this_coro {
struct [[nodiscard]] context_tag : tag_base {
    static constexpr auto operator()(context_payload auto const& payload) {
        return data_awaiter<co::context const&>{payload.context()};
    }
};

inline constexpr context_tag context{};
}
