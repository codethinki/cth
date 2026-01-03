#pragma once
#include "cth/coro/utility/fwd.hpp"


namespace cth::co::this_coro {
struct payload {
    constexpr payload(co::scheduler& scheduler) : _scheduler{&scheduler} {}

    constexpr co::scheduler& scheduler() const { return *_scheduler; }

private:
    co::scheduler* _scheduler;
};

}
