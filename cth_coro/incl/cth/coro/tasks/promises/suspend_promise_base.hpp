#pragma once

#include <coroutine>


namespace cth::co {
/**
 * promise part with a fixed initial and final awaiter
 * @tparam InitS initial awaiter
 * @tparam FinalS final awaiter
 */
template<auto InitS, auto FinalS>
struct suspend_promise_base {
    using initial_suspend_type = decltype(InitS);
    using final_suspend_type = decltype(FinalS);

    constexpr auto initial_suspend() noexcept { return InitS; }
    constexpr auto final_suspend() noexcept { return FinalS; }
};
}
