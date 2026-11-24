#pragma once
#include "cth/coro/sync.hpp"
#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/awaiters/schedule_awaiter.hpp"
#include "cth/coro/catching_scope.hpp"
#include "cth/coro/simple_counting_scope.hpp"
#include "cth/coro/tasks/dev/detach_task.hpp"
#include "cth/coro/tasks/dev/spawn_task.hpp"
#include "cth/coro/this_coro/payload.hpp"
#include "cth/coro/utility/concepts.hpp"


#include <cth/meta/coro.hpp>
#include <cth/ptr/ref_wrapper.hpp>

#include <type_traits>
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
     * @details enters by `context::dispatch()`: an idle strand resumes a caller in a job of its scheduler inline
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
     * - a caller in a job of this context runs it inline until its first suspension
     * - else it enters by `context::dispatch()`: an idle strand runs it inline too, else it is posted
     * - the scheduler of this context owns the task: `~scheduler()` destroys it if it did not end
     * - an exception of the task terminates
     * @tparam Pyld payload of the task, constructed from the arguments
     * @param task to run on this context
     * @param args to construct the payload with
     */
    template<payload Pyld = this_coro::default_payload, void_awaitable Awaitable, class... Args>
    void detach(Awaitable task, Args&&... args) const {
        detach_impl<Pyld, dev::no_scope, Awaitable>(
            dev::no_scope{},
            Pyld{std::forward<Args>(args)...},
            context(),
            std::move(task)
        ).start();
    }

    /**
     * runs the task on this context, without a caller, in the scope
     * @details
     * - the details of `detach()` without a scope, except the owner
     * - the scope owns the task: `~scheduler()` destroys only the tasks without a scope
     * - an exception of the task: `catching_scope` -> the scope stores it, else terminates
     * @pre the task ends before the scheduler of this context is destroyed: call `wait()` of the scope first
     * @tparam Pyld payload of the task, constructed from the arguments
     * @tparam Scope `simple_counting_scope` or `catching_scope`
     * @param scope the task is in
     * @param task to run on this context
     * @param args to construct the payload with
     */
    template<
        payload Pyld = this_coro::default_payload,
        detach_scope Scope,
        void_awaitable Awaitable,
        class... Args>
    void detach(Scope& scope, Awaitable task, Args&&... args) const {
        detach_impl<Pyld, Scope&, Awaitable>(
            scope,
            Pyld{std::forward<Args>(args)...},
            context(),
            std::move(task)
        ).start();
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

    // references, an empty tag and a local: the parameters of a coroutine are destroyed after its promise, the promise
    // leaves the scope
    /**
     * moves onto the context, then awaits the task
     * @tparam Scope `co::simple_counting_scope&`, `co::catching_scope&`, `dev::no_scope` -> no scope
     * @param scope the promise constructor reads it, the task enters it
     * @param payload the promise constructor reads it, the task inherits it
     * @param context the promise constructor reads it, the task runs on it
     * @param task to await, the coroutine moves it into its frame before `detach()` returns
     */
    template<payload Pyld, class Scope, awaitable Awaitable>
    static auto detach_impl(
        [[maybe_unused]] Scope scope,
        [[maybe_unused]] Pyld const& payload,
        co::context const& context,
        Awaitable&& task
    )
        -> dev::detach_task<Pyld, std::remove_reference_t<Scope>> {
        // the first statement: detach() holds the task until start() returns
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
