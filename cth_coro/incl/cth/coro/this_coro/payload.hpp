#pragma once
#include "cth/coro/utility/fwd.hpp"


namespace cth::co::this_coro {
struct payload {
    constexpr payload(co::scheduler const& scheduler) : _scheduler{&scheduler} {}

    constexpr co::scheduler const& scheduler() const { return *_scheduler; }

private:
    co::scheduler const* _scheduler;
};

}
