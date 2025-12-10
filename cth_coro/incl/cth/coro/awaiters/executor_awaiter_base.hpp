#pragma once
#include "cth/coro/utility/concepts.hpp"
#include <coroutine>


namespace cth::co {
/**
 * Base for awaitables which support executor injection
 */
struct executor_awaiter_base {
    executor_awaiter_base() = default;

    /**
     * Injects the executor into the current promise
     *
     * @tparam Promise type, must be @ref executor_injectable
     * @param h handle with promise to inject executor into
     */
    template<promise Promise>
    void inject_executor(std::coroutine_handle<Promise> h) {
        constexpr bool injectable = executor_injectable<Promise>;

        static_assert(
            injectable,
            "executor injected awaitables may not be awaited by awaitables without an executor"
        );


        if constexpr(injectable) h.promise().exec = exec;
    }

    executor* exec = nullptr;
};



}
