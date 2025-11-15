#pragma once
#include "cth/io/log.hpp"
#include "cth/types/typ_traits.hpp"

#include <coroutine>

namespace cth::co {
class executor;
}

namespace cth::co {

namespace this_coro {
    struct get_executor_t {};
    inline constexpr get_executor_t executor{};
}

class executor_awaiter {
public:
    executor_awaiter(executor& exec) : _exec(&exec) {}

    bool await_ready() const noexcept { return true; }
    void await_suspend(std::coroutine_handle<>) const noexcept {}
    executor& await_resume() const noexcept { return *_exec; }

private:
    executor* _exec;
};

}

namespace cth::co {
struct executor_promise_base;
}

namespace cth::co {

struct executor_awaiter_base {
    template<class Promise>
    void inject_executor(std::coroutine_handle<Promise> h) {
        if constexpr(std::is_base_of_v<executor_promise_base, Promise>) h.promise().exec = *exec;
    }

    executor* exec = nullptr;
};

template<class T>
concept executor_injected_awaitable = std::is_base_of_v<executor_awaiter_base, type::rcvr_t<T>>;

namespace dev {
    struct general_promise_base {
        executor* exec;

        general_promise_base() : exec{nullptr} {}
        general_promise_base(executor& e) : exec{&e} {}

        [[nodiscard]] executor_awaiter await_transform(this_coro::get_executor_t) {
            CTH_CRITICAL(
                exec == nullptr,
                "this must not happen and is a bug in the application, always call executor_awaiter_base::inject_executor"
            ) {}
            return {*exec};
        }

        template<executor_injected_awaitable Awaitable>
        [[nodiscard]] auto await_transform(Awaitable&& awaitable) const {
            awaitable.exec = exec;
            return std::forward<Awaitable>(awaitable);
        }
    };
}

struct executor_promise_base : dev::general_promise_base {
    using dev::general_promise_base::general_promise_base;
    template<class Awaitable>
    auto await_transform(Awaitable&& awaitable) = delete;
};

struct permissive_executor_promise_base : dev::general_promise_base {
    using dev::general_promise_base::general_promise_base;
};

}
