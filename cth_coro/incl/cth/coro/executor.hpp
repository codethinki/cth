#pragma once
#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/awaiters/schedule_awaiter.hpp"
#include "cth/coro/func/steal.hpp"
#include "cth/coro/tasks/dev/scheduled_task.hpp"
#include "cth/coro/utility/concepts.hpp"


#include <cth/meta/coro.hpp>


namespace cth::co {
class executor {


public:
    executor(scheduler const& sched) noexcept : _sched(&sched) {}
    ~executor() = default;

    [[nodiscard]] constexpr auto schedule() const { return schedule_awaiter{scheduler()}; }


    template<this_coro_awaitable Awaitable>
    auto steal(Awaitable&& awaitable) -> awaiter_t<Awaitable> {
        return co::steal(scheduler(), awaitable);
    }

    template<non_this_coro_awaitable Awaitable>
    auto steal(Awaitable awaitable) -> capture_task<awaited_t<Awaitable>> {
        return co::steal(scheduler(), std::move(awaitable));
    }


    template<class T>
    auto spawn(scheduled_task<T> task) -> scheduled_task<T> {
        return executor::steal(std::move(task));
    }

    template<awaitable Awaitable>
    auto spawn(Awaitable task) -> scheduled_task<awaited_t<Awaitable>> {
        return executor::spawn(this_coro::payload{scheduler()}, *this, std::move(task));
    }

private:
    template<awaitable Awaitable>
    static auto spawn([[maybe_unused]] this_coro::payload p, executor& s, Awaitable task)
        -> scheduled_task<awaited_t<Awaitable>> {
        co_await s.schedule();
        co_return co_await s.steal(std::move(task));
    }

    scheduler const* _sched;

public:
    co::scheduler const& scheduler() const { return *_sched; }

    bool operator==(executor const&) const = default;

    executor(executor const& other) = default;
    executor(executor&& other) noexcept = default;
    executor& operator=(executor const& other) = default;
    executor& operator=(executor&& other) noexcept = default;
};


}


namespace cth::co::this_coro {
struct [[nodiscard]] executor_tag : tag_base {
    static auto operator()(payload const& p) { return data_awaiter{executor{p.scheduler()}}; }
};


inline constexpr executor_tag executor{};

}
