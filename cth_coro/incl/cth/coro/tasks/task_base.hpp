#pragma once
#include "cth/coro/utility/concepts.hpp"

#include <cth/coro/unique_cohandle.hpp>

#include <coroutine>

namespace cth::co {
/**
 * base of the tasks, owns the coroutine
 * @tparam Promise promise of the task
 * @tparam Awaiter awaiter template, @ref operator co_await() gives an Awaiter<Promise>
 */
template<cth_promise Promise, template<class> class Awaiter>
class [[nodiscard]] task_base {
public:
    using promise_type = Promise;
    using awaiter_type = Awaiter<Promise>;


    constexpr ~task_base() = default;

protected:
    constexpr task_base(std::coroutine_handle<promise_type> h) noexcept : _handle{h} {}

private:
    unique_cohandle<promise_type> _handle;

public:
    /**
     * gives the coroutine, the task stays the owner
     */
    [[nodiscard]] constexpr auto handle(this auto&& self) { return self._handle.get(); }

    /**
     * gives the awaiter of the task, the task stays the owner of the coroutine
     * @details a temporary task lives until the end of the full expression, after await_resume()
     * @pre the task owns a coroutine: not moved from
     * @attention the task must outlive the co_await
     */
    [[nodiscard]] constexpr auto operator co_await() const& noexcept { return awaiter_type{handle()}; }

    task_base(task_base const&) = delete;
    task_base& operator=(task_base const&) = delete;
    constexpr task_base(task_base&&) noexcept = default;
    constexpr task_base& operator=(task_base&&) noexcept = default;
};

}
