#pragma once
#include "cth/coro/concepts.hpp"
#include "cth/coro/tasks/dev/capture_task.hpp"


#include <cth/coro/utility.hpp>
#include <cth/types/coro.hpp>

#include <utility>

namespace cth::co {
class executor;


template<class Awaitable>
auto steal(executor& exec, Awaitable&&) -> capture_task<awaited_t<Awaitable>>;


struct executor_promise_base {
    executor* exec = nullptr;

    template<executor_awaitable Awaitable>
    [[nodiscard]] auto await_transform(Awaitable&& awaitable) -> awaiter_t<Awaitable> {
        auto awaiter = extract_awaiter(awaitable);

        if(awaiter.exec == nullptr)
            awaiter.exec = exec;
        return awaiter;
    }

    template<captured_awaitable Awaitable>
    [[nodiscard]] auto await_transform(Awaitable&& captured) { return cth::co::extract_awaiter(captured); }

    template<class Awaitable>
    auto await_transform(Awaitable&& awaitable) -> capture_task<awaited_t<Awaitable>> {
        return co::steal(*exec, std::forward<Awaitable>(awaitable));
    }
};

}
