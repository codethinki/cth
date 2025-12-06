#pragma once
#include <cth/types/coro.hpp>

#include <coroutine>
#include <type_traits>

namespace cth::co {
class executor;
struct executor_promise_base;
}

namespace cth::co {

template<class Promise>
concept executor_injectable = std::is_base_of_v<executor_promise_base, Promise>;

/**
 * Base for awaitables which support executor injection
 */
struct executor_awaiter_base {
    executor_awaiter_base() = default;

    /**
     * Injects the executor into the callers promise
     *
     * @tparam Promise type, must be @ref executor_injectable
     * @param h handle with promise to inject executor into
     */
    template<class Promise>
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

template<class T>
concept executor_awaitable = std::is_base_of_v<executor_awaiter_base, type::rcvr_t<awaiter_t<T>>>;

}
