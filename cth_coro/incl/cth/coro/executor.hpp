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
     * calling co_await on this switches the execution context to this scheduler and stays there
     * @tparam Pyld payload the caller must carry, defaulted to @ref this_coro::default_payload
     */
    template<payload Pyld = this_coro::default_payload>
    [[nodiscard]] constexpr auto schedule() const { return schedule_awaiter<Pyld>{scheduler()}; }


    /**
     * steals the execution context to the scheduler after the coroutine finished, keeps it as awaitable
     * @tparam Pyld payload type; the scheduler is injected automatically when it is a scheduler_payload
     * @tparam Awaitable must be compatible with the this_coro framework
     * @param awaitable to steal context from
     * @param args extra arguments to construct the payload with (besides the auto-injected scheduler)
     * @return awaitable with context stolen
     */
    template<payload Pyld = this_coro::default_payload, this_coro_awaitable Awaitable, class... Args>
    [[nodiscard]] auto steal(Awaitable&& awaitable, Args&&... args) const -> awaiter_t<Awaitable> {
        if constexpr(this_coro::scheduler_payload<Pyld>)
            return co::steal(std::forward<Awaitable>(awaitable), Pyld{scheduler(), std::forward<Args>(args)...});
        else
            return co::steal(std::forward<Awaitable>(awaitable), Pyld{std::forward<Args>(args)...});
    }

    /**
     * steals the execution context to the scheduler after the coroutine finished
     * @note converts the awaitable to a task
     * @tparam Pyld payload type; the scheduler is injected automatically when it is a scheduler_payload
     * @tparam Awaitable must not be compatible with the this_coro framework
     * @param awaitable to steal context from
     * @param args extra arguments to construct the payload with (besides the auto-injected scheduler)
     * @return task with context stolen
     */
    template<payload Pyld = this_coro::default_payload, non_this_coro_awaitable Awaitable, class... Args>
    [[nodiscard]] auto steal(Awaitable awaitable, Args&&... args) const -> capture_task<awaited_t<Awaitable>> {
        if constexpr(this_coro::scheduler_payload<Pyld>)
            return co::steal(std::move(awaitable), Pyld{scheduler(), std::forward<Args>(args)...});
        else
            return co::steal(std::move(awaitable), Pyld{std::forward<Args>(args)...});
    }

    /**
     * runs the coroutine on the scheduler once started, then captures execution back
     * @tparam Pyld payload type; the scheduler is injected automatically when it is a scheduler_payload
     * @param task coroutine to run on the scheduler
     * @param args extra arguments to construct the payload with (besides the auto-injected scheduler)
     * @return cold scheduled task; starts on the scheduler when awaited
     */
    template<payload Pyld = this_coro::default_payload, awaitable Awaitable, class... Args>
    [[nodiscard]] auto spawn(Awaitable task, Args&&... args) const -> scheduled_task<awaited_t<Awaitable>, Pyld> {
        if constexpr(this_coro::scheduler_payload<Pyld>)
            return spawn_impl(Pyld{scheduler(), std::forward<Args>(args)...}, *this, std::move(task));
        else
            return spawn_impl(Pyld{std::forward<Args>(args)...}, *this, std::move(task));
    }

private:
    template<payload Pyld, awaitable Awaitable>
    static auto spawn_impl(
        Pyld payload,
        executor self,
        Awaitable task
    )
        -> scheduled_task<awaited_t<Awaitable>, Pyld> {
        co_await self.schedule<Pyld>();

        // inject the payload into the child and finish back on this executor
        co_return co_await co::steal(std::move(task), payload);
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
    [[nodiscard]] static constexpr auto operator()(default_payload const& p) {
        return data_awaiter{executor{p.scheduler()}};
    }
};


inline constexpr executor_tag executor{};

}
