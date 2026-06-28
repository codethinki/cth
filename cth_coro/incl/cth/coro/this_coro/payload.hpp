#pragma once
#include "cth/coro/utility/fwd.hpp"


namespace cth::co::this_coro {
struct scheduler_payload_base {
    constexpr scheduler_payload_base(co::scheduler const& scheduler) : _scheduler{&scheduler} {}

    constexpr co::scheduler const& scheduler() const { return *_scheduler; }

private:
    co::scheduler const* _scheduler;
};

template<class Pyld>
concept scheduler_payload = std::is_base_of_v<scheduler_payload_base, std::remove_cvref_t<Pyld>>;

}
