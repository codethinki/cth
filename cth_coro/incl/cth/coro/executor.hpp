#pragma once
#include "cth/coro/concepts.hpp"
#include "cth/coro/awaiters/schedule_awaiter.hpp"
#include "cth/coro/func/steal.hpp"
#include "cth/coro/tasks/dev/scheduled_task.hpp"

#include <cth/types/coro.hpp>

namespace cth::co {
class executor {

public:
    explicit executor(scheduler& sched) noexcept : _sched(&sched) {}
    ~executor() = default;

    auto schedule() const { return schedule_awaiter{*_sched}; }


    template<executor_awaitable Awaitable>
    auto steal(Awaitable&& awaitable) -> awaiter_t<Awaitable> {
        auto awaiter = co::extract_awaiter(std::forward<Awaitable>(awaitable));
        awaiter.exec = this;
        return awaiter;
    }

    template<executorless_awaitable Awaitable>
    auto steal(Awaitable awaitable) -> capture_task<awaited_t<Awaitable>> {
        return co::steal(*_sched, std::move(awaitable));
    }

    template<class T>
    auto spawn(scheduled_task<T> task) -> scheduled_task<T> {
        return executor::steal(std::move(task));
    }

    template<awaitable Awaitable>
    auto spawn(Awaitable task) -> scheduled_task<awaited_t<Awaitable>> {
        co_await schedule();
        co_return co_await executor::steal(std::move(task));
    }

private:
    scheduler* _sched;

public:
    executor(executor const& other) = default;
    executor(executor&& other) noexcept = default;
    executor& operator=(executor const& other) = default;
    executor& operator=(executor&& other) noexcept = default;
};


template<foreign_awaitable Awaitable>
auto steal(executor& exec, Awaitable awaitable) -> capture_task<awaited_t<Awaitable>> {
    return exec.steal(std::move(awaitable));
}

}
