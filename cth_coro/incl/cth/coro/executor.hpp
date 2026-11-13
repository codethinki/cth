#pragma once
#include "cth/coro/sync.hpp"
#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/awaiters/schedule_awaiter.hpp"
#include "cth/coro/simple_counting_scope.hpp"
#include "cth/coro/tasks/dev/detach_task.hpp"
#include "cth/coro/tasks/dev/spawn_task.hpp"
#include "cth/coro/this_coro/payload.hpp"
#include "cth/coro/utility/concepts.hpp"


#include <cth/meta/coro.hpp>
#include <cth/ptr/ref_wrapper.hpp>

#include <utility>


namespace cth::co {
/**
 * spawns tasks on a context and moves coroutines onto it
 * @details holds a pointer to the context, copies refer to the same context
 * @attention the context must outlive the executor and its tasks
 */
class executor {
public:
    constexpr executor(co::context const& context) noexcept : _context{context} {}
    constexpr ~executor() = default;

    /**
     * moves the caller onto this context
     * @details a `this_coro` caller continues on this context after later `co_await`s too
     * @details the payload of the caller stays
     * @return awaiter, `co_await` it
     */
    [[nodiscard]] constexpr auto schedule() const { return schedule_awaiter{context()}; }

    /**
     * runs the task on this context once awaited, with a new payload
     * @details the caller continues on its context, a caller without a context continues on this context
     * @tparam Pyld payload of the task, constructed from the arguments
     * @param task to run on this context
     * @param args to construct the payload with
     * @return spawn task, starts when awaited
     */
    template<payload Pyld = this_coro::default_payload, awaitable Awaitable, class... Args>
    [[nodiscard]] auto spawn(Awaitable task, Args&&... args) const -> spawn_task<awaited_t<Awaitable>, Pyld> {
        return spawn_impl(Pyld{std::forward<Args>(args)...}, context(), std::move(task));
    }

    /**
     * spawns the task and blocks until it ends
     * @tparam Pyld payload of the task, constructed from the arguments
     * @param task to run on this context
     * @param args to construct the payload with
     * @return result of the task
     * @throws what the task throws
     * @attention blocks the calling thread
     * @attention called in a job of this context: deadlock once the task suspends (a strand job, the only worker)
     */
    template<payload Pyld = this_coro::default_payload, awaitable Awaitable, class... Args>
    auto sync_wait(Awaitable task, Args&&... args) const -> awaited_t<Awaitable> {
        return co::sync(spawn<Pyld>(std::move(task), std::forward<Args>(args)...));
    }

    /**
     * runs the task on this context, without a caller
     * @details
     * - runs on the calling thread until it moves onto this context, see `schedule()`
     * - a caller in a job of this context runs it inline until its first suspension
     * - the scheduler of this context owns the task until it ends, see `context::detach()`
     * - the task did not end at `~scheduler()` -> the scheduler destroys it
     * - the coroutine of the task is destroyed -> leaves the scope
     * - an exception of the task terminates
     * @tparam Pyld payload of the task, constructed from the arguments
     * @param scope the task enters, `simple_counting_scope::wait()` waits for it
     * @param task to run on this context
     * @param args to construct the payload with
     */
    template<payload Pyld = this_coro::default_payload, void_awaitable Awaitable, class... Args>
    void detach(co::simple_counting_scope& scope, Awaitable task, Args&&... args) const {
        detach_impl<Pyld, Awaitable>(scope, Pyld{std::forward<Args>(args)...}, context(), std::move(task));
    }

private:
    /**
     * moves onto the context, then awaits the task
     * @param payload the promise constructor reads it, the task inherits it
     * @param context the promise constructor reads it, the task runs on it
     * @param task to await
     */
    template<payload Pyld, awaitable Awaitable>
    static auto spawn_impl(
        [[maybe_unused]] Pyld payload,
        co::context const& context,
        Awaitable task
    )
        -> spawn_task<awaited_t<Awaitable>, Pyld> {
        co_await schedule_awaiter{context};
        co_return co_await std::move(task);
    }

    // references and a local: the parameters of a coroutine are destroyed after its promise, the promise leaves the
    // scope
    /**
     * moves onto the context, then awaits the task
     * @details the parameters are references, the frame holds the task in a local
     * @param scope the promise constructor reads it, the task enters it
     * @param payload the promise constructor reads it, the task inherits it
     * @param context the promise constructor reads it, the task runs on it
     * @param task to await, the coroutine moves it into its frame before its first suspension
     */
    template<payload Pyld, awaitable Awaitable>
    static auto detach_impl(
        [[maybe_unused]] co::simple_counting_scope& scope,
        [[maybe_unused]] Pyld const& payload,
        co::context const& context,
        Awaitable&& task
    )
        -> dev::detach_task<Pyld> {
        // before the first suspension: the caller of detach() holds the task until then
        Awaitable ownedTask = std::move(task);
        co_await schedule_awaiter{context};
        co_await std::move(ownedTask);
    }

    cth::ref_wrapper<co::context const> _context;

public:
    constexpr co::context const& context() const { return *_context; }

    /**
     * checks if both executors refer to the same context
     */
    constexpr bool operator==(executor const& other) const { return &context() == &other.context(); }

    constexpr executor(executor const& other) = default;
    constexpr executor(executor&& other) noexcept = default;
    constexpr executor& operator=(executor const& other) = default;
    constexpr executor& operator=(executor&& other) noexcept = default;
};


}

namespace cth::co::this_coro {
/**
 * tag of `this_coro::executor`
 */
struct [[nodiscard]] executor_tag : tag_base {
    /**
     * gives an executor of the context of the caller
     * @param c context of the caller
     * @return ready awaiter with the executor
     */
    [[nodiscard]] static constexpr auto operator()(co::context const& c, auto const&) {
        return data_awaiter{executor{c}};
    }
};


/**
 * `co_await` it: gives an executor of the context of the caller
 */
inline constexpr executor_tag executor{};

}
