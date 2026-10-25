#pragma once
#include "cth/coro/tasks/dev/capture_task.hpp"
#include "cth/coro/utility/concepts.hpp"

#include <cth/meta/coro.hpp>

#include <utility>

namespace cth::co::dev {
/**
 * captures a foreign awaitable
 * @details the capture continues the caller on its scheduler, also after an exception
 * @param awaitable to capture
 * @return capture task, awaitable by this_coro coroutines
 */
template<foreign_awaitable Awaitable>
[[nodiscard]] auto steal(Awaitable awaitable) -> capture_task<awaited_t<Awaitable>> {
    co_return co_await std::move(awaitable); // void: co_await, then co_return
}

}
