#pragma once
#include "cth/coro/sync.hpp"
#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/awaiters/schedule_awaiter.hpp"
#include "cth/coro/tasks/dev/spawn_task.hpp"
#include "cth/coro/this_coro/payload.hpp"
#include "cth/coro/utility/concepts.hpp"


#include <cth/io/log.hpp>
#include <cth/meta/coro.hpp>

#include <utility>


namespace cth::co {
/**
 * wraps the scheduler with coroutines functionality
 */
class executor {
public:
    constexpr executor(co::scheduler const& sched) noexcept : _sched(&sched) {}
    constexpr ~executor() = default;

    /**
     * calling co_await on this switches the execution context to this scheduler and stays there
     * @details the payload of the caller stays
     */
    [[nodiscard]] constexpr auto schedule() const { return schedule_awaiter{scheduler()}; }

    /**
     * runs the task on this scheduler once awaited, with a fresh payload
     * @details the caller continues on its scheduler, a caller without a scheduler continues on this scheduler
     * @tparam Pyld payload of the task, constructed from the arguments
     * @param task to run on this scheduler
     * @param args to construct the payload with
     * @return cold spawn task, starts when awaited
     */
    template<payload Pyld = this_coro::default_payload, awaitable Awaitable, class... Args>
    [[nodiscard]] auto spawn(Awaitable task, Args&&... args) const -> spawn_task<awaited_t<Awaitable>, Pyld> {
        return spawn_impl(Pyld{std::forward<Args>(args)...}, *this, std::move(task));
    }

    /**
     * spawns the task and blocks until it ends
     * @tparam Pyld payload of the task, constructed from the arguments
     * @param task to run on this scheduler
     * @param args to construct the payload with
     * @return result of the task
     * @pre @ref scheduler() does not own the calling thread
     * @attention blocks the calling thread
     */
    template<payload Pyld = this_coro::default_payload, awaitable Awaitable, class... Args>
    auto sync_wait(Awaitable task, Args&&... args) const -> awaited_t<Awaitable> {
        CTH_CRITICAL(scheduler().owns_thread(), "sync_wait blocks a worker of its own scheduler") {}
        return co::sync(spawn<Pyld>(std::move(task), std::forward<Args>(args)...));
    }

private:
    /**
     * @param payload the promise constructor reads it, the task inherits it
     * @param self the promise constructor reads it, the task runs on its scheduler
     */
    template<payload Pyld, awaitable Awaitable>
    static auto spawn_impl(
        [[maybe_unused]] Pyld payload,
        executor self,
        Awaitable task
    )
        -> spawn_task<awaited_t<Awaitable>, Pyld> {
        co_await self.schedule();
        co_return co_await std::move(task);
    }

    co::scheduler const* _sched;

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

namespace cth::co::dev {
template<class T, payload Pyld>
template<class... Args>
spawn_promise<T, Pyld>::spawn_promise(Pyld const& payload, executor const& self, Args&&...) :
    this_coro_promise_base<Pyld>{self.scheduler(), payload} {}
}


namespace cth::co::this_coro {
struct [[nodiscard]] executor_tag : tag_base {
    [[nodiscard]] static constexpr auto operator()(co::scheduler const& s, auto const&) {
        return data_awaiter{executor{s}};
    }
};


inline constexpr executor_tag executor{};

}
