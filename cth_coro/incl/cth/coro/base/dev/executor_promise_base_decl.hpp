#pragma once
#include "cth/coro/base/executor_awaiter_base.hpp"
#include "cth/coro/base/dev/capture_awaiter_base.hpp"
#include "cth/coro/dev/capture_task.hpp"

#include <cth/coro.hpp>
#include <cth/types/coro.hpp>


namespace cth::co {

struct executor_promise_base {
    executor* exec = nullptr;

    template<executor_awaitable Awaitable>
    [[nodiscard]] auto await_transform(Awaitable&& awaitable) -> awaiter_t<Awaitable> {
        auto awaiter = extract_awaiter(awaitable);

        if(awaiter.exec == nullptr)
            awaiter.exec = exec;
        return awaiter;
    }

    template<dev::captured_awaitable Awaitable>
    [[nodiscard]] auto await_transform(Awaitable&& captured) { return extract_awaiter(captured); }

    template<class Awaitable>
    auto await_transform(Awaitable&& awaitable) -> capture_task<awaited_t<Awaitable>>;
};

}
