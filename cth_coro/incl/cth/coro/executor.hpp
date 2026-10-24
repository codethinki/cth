#pragma once
#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/awaiters/schedule_awaiter.hpp"
#include "cth/coro/func/steal.hpp"
#include "cth/coro/tasks/dev/spawn_task.hpp"
#include "cth/coro/utility/concepts.hpp"


#include <cth/meta/coro.hpp>


namespace cth::co {
/**
 * wraps the context with coroutines functionality
 */
class executor {
public:
    constexpr executor(co::context const& context) noexcept : _context(&context) {}
    constexpr ~executor() = default;

    /**
     * calling co_await on this switches the execution onto this context and stays there
     * @tparam Pyld payload the caller must carry, defaulted to @ref this_coro::default_payload
     */
    template<payload Pyld = this_coro::default_payload>
    [[nodiscard]] constexpr auto schedule() const { return schedule_awaiter<Pyld>{context()}; }


    /**
     * steals the execution to the context after the coroutine finished, keeps it as awaitable
     * @tparam Pyld payload type; the context is injected automatically when it is a context_payload
     * @tparam Awaitable must be compatible with the this_coro framework
     * @param awaitable to steal context from
     * @param args extra arguments to construct the payload with (besides the auto-injected context)
     * @return awaitable with context stolen
     * @attention the awaiter does not own the task, co_await it in the same expression, do not store it
     */
    template<payload Pyld = this_coro::default_payload, this_coro_awaitable Awaitable, class... Args>
    [[nodiscard]] constexpr auto steal(Awaitable&& awaitable, Args&&... args) const -> awaiter_t<Awaitable> {
        if constexpr(this_coro::context_payload<Pyld>)
            return co::steal(std::forward<Awaitable>(awaitable), Pyld{context(), std::forward<Args>(args)...});
        else
            return co::steal(std::forward<Awaitable>(awaitable), Pyld{std::forward<Args>(args)...});
    }

    /**
     * steals the execution to the context after the coroutine finished
     * @note converts the awaitable to a task
     * @tparam Pyld payload type; the context is injected automatically when it is a context_payload
     * @tparam Awaitable must not be compatible with the this_coro framework
     * @param awaitable to steal context from
     * @param args extra arguments to construct the payload with (besides the auto-injected context)
     * @return task with context stolen
     */
    template<payload Pyld = this_coro::default_payload, non_this_coro_awaitable Awaitable, class... Args>
    [[nodiscard]] auto steal(Awaitable awaitable, Args&&... args) const -> capture_task<awaited_t<Awaitable>> {
        if constexpr(this_coro::context_payload<Pyld>)
            return co::steal(std::move(awaitable), Pyld{context(), std::forward<Args>(args)...});
        else
            return co::steal(std::move(awaitable), Pyld{std::forward<Args>(args)...});
    }

    /**
     * runs the coroutine on the context once started
     * @details the caller continues on its own context afterwards, with one post if the coroutine ended elsewhere,
     *  a caller without this_coro support continues where the coroutine ended
     * @tparam Pyld payload type; the context is injected automatically when it is a context_payload
     * @param task coroutine to run on the context
     * @param args extra arguments to construct the payload with (besides the auto-injected context)
     * @return cold spawn task; starts on the context when awaited
     */
    template<payload Pyld = this_coro::default_payload, awaitable Awaitable, class... Args>
    [[nodiscard]] auto spawn(Awaitable task, Args&&... args) const -> spawn_task<awaited_t<Awaitable>> {
        if constexpr(this_coro::context_payload<Pyld>)
            return spawn_impl(Pyld{context(), std::forward<Args>(args)...}, *this, std::move(task));
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
        -> spawn_task<awaited_t<Awaitable>> {
        co_await self.schedule();

        // the final awaiter of the spawn_task resumes the caller on its own context
        if constexpr(this_coro_awaitable<Awaitable>)
            co_return co_await co::steal(std::move(task), payload); // injects the payload, no hop
        else
            co_return co_await task;
    }

    co::context const* _context;

public:
    /**
     * gets underlying context
     */
    constexpr co::context const& context() const { return *_context; }

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
        return data_awaiter{executor{p.context()}};
    }
};


inline constexpr executor_tag executor{};

}
