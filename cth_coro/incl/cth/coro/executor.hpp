#pragma once
#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/awaiters/schedule_awaiter.hpp"
#include "cth/coro/func/steal.hpp"
#include "cth/coro/tasks/dev/scheduled_task.hpp"
#include "cth/coro/utility/concepts.hpp"


#include <cth/meta/coro.hpp>


namespace cth::co {
/**
 * wraps the scheduler with coroutines functionality
 */
class executor {
public:
    constexpr executor(scheduler const& sched) noexcept : _sched(&sched) {}
    constexpr ~executor() = default;

    /**
     * calling co_await on this will switch the execution context to this scheduler
     * @details must already be this_coro compatible
     */
    [[nodiscard]] constexpr auto schedule() const { return schedule_awaiter{scheduler()}; }


    /**
     * steals the execution context to the scheduler, keeps it as awaitable
     * @tparam Awaitable must be compatible with the this_coro framework
     * @param awaitable to steal context from
     * @return awaitable with context stolen
     */
    template<this_coro_awaitable Awaitable>
    auto steal(Awaitable&& awaitable) -> awaiter_t<Awaitable> { return co::steal(scheduler(), awaitable); }

    /**
     * steals the execution context to the scheduler
     * @note converts the awaitable to a task
     * @tparam Awaitable must not be compatible with the this_coro framework
     * @param awaitable to steal context from
     * @return task with context stolen
     */
    template<non_this_coro_awaitable Awaitable>
    auto steal(Awaitable awaitable) -> capture_task<awaited_t<Awaitable>> {
        return co::steal(scheduler(), std::move(awaitable));
    }


    /**
     * runs the task on the scheduler once started
     * @tparam T type to return
     * @param task to run
     */
    template<class T>
    auto spawn(scheduled_task<T> task) -> scheduled_task<T> { return executor::steal(std::move(task)); }

    /**
     * wraps the awaitable with a task running on the scheduler once started
     * @param awaitable to wrap
     * @return task
     */
    template<awaitable Awaitable>
    auto spawn(Awaitable awaitable) -> scheduled_task<awaited_t<Awaitable>> {
        return executor::spawn(this_coro::payload{scheduler()}, *this, std::move(awaitable));
    }

private:
    template<awaitable Awaitable>
    static auto spawn(
        [[maybe_unused]] this_coro::payload p,
        executor& s,
        Awaitable task
    )
        -> scheduled_task<awaited_t<Awaitable>> {
        co_await s.schedule();
        co_return co_await s.steal(std::move(task));
    }

    scheduler const* _sched;

public:
    /**
     * gets underlying scheduler
     */
    constexpr co::scheduler const& scheduler() const { return *_sched; }

    constexpr bool operator==(executor const&) const = default;

    constexpr executor(executor const& other) = default;
    constexpr executor(executor&& other) noexcept = default;
    constexpr executor& operator=(executor const& other) = default;
    constexpr executor& operator=(executor&& other) noexcept = default;
};


}


namespace cth::co::this_coro {
struct [[nodiscard]] executor_tag : tag_base {
    static [[nodiscard]] constexpr auto operator()(payload const& p) { return data_awaiter{executor{p.scheduler()}}; }
};


inline constexpr executor_tag executor{};

}
