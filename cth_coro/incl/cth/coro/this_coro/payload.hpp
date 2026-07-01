#pragma once
#include "cth/coro/utility/fwd.hpp"


namespace cth::co::this_coro {
/**
 * class base for payloads carrying a scheduler
 * @details this coro machinery will auto-inject the scheduler for types inheriting this base
 */
struct scheduler_payload_base {
    constexpr scheduler_payload_base(co::scheduler const& scheduler) : _scheduler{&scheduler} {}

    constexpr co::scheduler const& scheduler() const { return *_scheduler; }

    /**
     * framework use: repoint the payload at the scheduler the coroutine has migrated onto
     * @details keeps every derived field intact, only the scheduler is swapped
     * @param scheduler the coroutine is now running on
     */
    constexpr void rebind_scheduler(co::scheduler const& scheduler) noexcept { _scheduler = &scheduler; }

private:
    co::scheduler const* _scheduler;
};

}
