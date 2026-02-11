#pragma once
#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/utility/fwd.hpp"
#include "payload.hpp"


namespace cth::co::this_coro {
struct [[nodiscard]] scheduler_tag : tag_base {
    static constexpr auto operator()(payload const& payload) {
        return data_awaiter<co::scheduler const&>{payload.scheduler()};
    }
};

inline constexpr scheduler_tag scheduler{};
}
