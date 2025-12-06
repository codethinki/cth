#pragma once
#include "scheduler.hpp"
#include "schedule_awaiter.hpp"
#include "dev/executor_task_decl.hpp"
#include "dev/scheduled_task.hpp"

#include "cth/coro/func/steal.hpp"
#include "cth/types/coro.hpp"

namespace cth::co {
class executor {

public:
    explicit executor(scheduler& sched) noexcept : _sched(&sched) {}
    ~executor() = default;

    auto schedule() const { return schedule_awaiter{*_sched}; }


    template<executor_awaitable Awaitable>
    auto steal(Awaitable&& awaitable) -> awaiter_t<Awaitable> {
        auto awaiter = extract_awaiter(std::forward<Awaitable>(awaitable));
        awaiter.exec = this;
        return awaiter;
    }

    template<class Awaitable>
    auto steal(Awaitable&& awaitable) { return co::steal(*_sched, std::forward<Awaitable>(awaitable)); }



    template<class Awaitable>
    auto spawn(Awaitable task) -> scheduled_task<awaited_t<Awaitable>> {
        co_await schedule();
        co_return co_await executor::steal(task);
    }

private:
    scheduler* _sched;

public:
    executor(executor const& other) = default;
    executor(executor&& other) noexcept = default;
    executor& operator=(executor const& other) = default;
    executor& operator=(executor&& other) noexcept = default;
};


}
