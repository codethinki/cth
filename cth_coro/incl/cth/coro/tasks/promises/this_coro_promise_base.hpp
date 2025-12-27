#pragma once
#include "cth/coro/this_coro.hpp"
#include "cth/coro/func/steal.hpp"
#include "cth/coro/tasks/dev/capture_task.hpp"
#include "cth/coro/utility/concepts.hpp"

#include <cth/coro/utility.hpp>
#include <cth/meta/coro.hpp>

#include <utility>

namespace cth::co {
class executor;


struct this_coro_promise_base {

    template<this_coro::tag T>
    [[nodiscard]] decltype(auto) await_transform(T&& t) {
        return std::forward<T>(t)(*_payload);
    }


    template<this_coro_awaitable Awaitable>
    [[nodiscard]] auto await_transform(Awaitable&& awaitable) -> awaiter_t<Awaitable> {
        auto awaiter = co::extract_awaiter(awaitable);

        awaiter.inject(*_payload);

        return awaiter;
    }

    template<captured_awaitable Awaitable>
    [[nodiscard]] auto await_transform(Awaitable&& captured) { return cth::co::extract_awaiter(captured); }

    template<foreign_awaitable Awaitable>
    [[nodiscard]] auto await_transform(Awaitable awaitable) -> capture_task<awaited_t<Awaitable>> {
        return co::steal(_payload->scheduler(), std::move(awaitable));
    }

    /**
     * injects the this_coro payload into the promise
     * @param payload to inject
     */
    void inject(this_coro::payload payload) { _payload = payload; }

    this_coro_promise_base(this_coro::payload payload) noexcept : _payload{std::move(payload)} {}
    this_coro_promise_base() noexcept = default;

private:
    std::optional<this_coro::payload> _payload{};
};

}
