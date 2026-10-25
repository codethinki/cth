#pragma once
#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/utility/fwd.hpp"


namespace cth::co::this_coro {
struct [[nodiscard]] scheduler_tag : tag_base {
    static constexpr auto operator()(co::scheduler const& s, auto const&) {
        return data_awaiter<co::scheduler const&>{s};
    }
};

inline constexpr scheduler_tag scheduler{};
}
