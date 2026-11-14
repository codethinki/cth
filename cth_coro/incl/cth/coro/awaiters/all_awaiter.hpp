#pragma once
#include "cth/coro/exception.hpp"
#include "cth/coro/tasks/dev/all_child_task.hpp"
#include "cth/coro/this_coro/this_coro_base.hpp"
#include "cth/coro/utility/concepts.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/coro/utility.hpp>
#include <cth/meta/concepts.hpp>
#include <cth/meta/coro.hpp>
#include <cth/ptr/ref_wrapper.hpp>

#include <coroutine>
#include <cstddef>
#include <exception>
#include <ranges>
#include <source_location>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>


namespace cth::co::dev {
/**
 * starts the children of `co::all()`, owns their coroutines
 * @details the caller resumes after the last child on its context
 */
class all_awaiter_base : public this_coro_awaiter_base {
public:
    /**
     * the children start in `await_suspend()`
     */
    [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }

protected:
    all_awaiter_base() = default;

    /**
     * destroys the coroutines of the children
     * @pre every started child ended
     */
    ~all_awaiter_base() {
        for(auto const child : _children)
            child.destroy();
    }

    /**
     * a ready awaiter: stores its result at once, else creates the child that awaits it
     * @details the child does not start yet, see `start()`
     * @tparam T result type of the awaitable
     * @param caller coroutine of the caller, the child gets its context and its payload
     * @param awaitable to await, lives until the end of the `co_await`
     * @param[out] slot gets the result or the exception
     * @pre `reserve()` for the child
     * @throws what the allocation of the coroutine throws
     */
    template<class T, class Caller, awaitable Awaitable>
    void add(std::coroutine_handle<Caller> caller, Awaitable&& awaitable, all_slot<T>& slot) {
        using awaiter_type = decltype(co::extract_awaiter(std::forward<Awaitable>(awaitable)));
        awaiter_type awaiter = co::extract_awaiter(std::forward<Awaitable>(awaitable));

        try {
            if(awaiter.await_ready()) {
                if constexpr(mta::is_void<T>) {
                    awaiter.await_resume();
                    slot.result.emplace();
                } else
                    slot.result.emplace(awaiter.await_resume());
                return;
            }
        }
        catch(...) {
            slot.exception = std::current_exception();
            return;
        }

        auto const& promise = caller.promise();
        auto const child = all_child<T, payload_t<Caller>, awaiter_type>(
            _state,
            promise.context(),
            promise.payload(),
            std::forward<awaiter_type>(awaiter),
            slot
        );
        _children.push_back(child.handle);
    }

    /**
     * makes room for the children, before the first `add()`
     */
    void reserve(size_t children) { _children.reserve(children); }

    /**
     * starts the children in turn on the calling thread, each runs until it suspends
     * @param caller coroutine of the caller
     * @return `false` -> every child ended, the caller does not suspend
     */
    template<this_coro_compatible Caller>
    [[nodiscard]] bool start(std::coroutine_handle<Caller> caller) {
        _state.caller = caller;
        _state.callerContext = &caller.promise().context();
        // one more for the start: no child resumes the caller before every child started
        _state.count.store(_children.size() + 1, std::memory_order_relaxed);

        for(auto const child : _children)
            child.resume();

        // the start is the last: the children ended
        return _state.count.fetch_sub(1, std::memory_order_acq_rel) != 1;
    }

    /**
     * throws if a child failed
     * @param exceptions one per child in child order
     * @throws cth::except::group_coro_exception reason: a child failed
     */
    static void throw_failures(
        std::vector<std::exception_ptr> exceptions,
        std::source_location const& loc = std::source_location::current()
    ) {
        for(auto const& exception : exceptions)
            if(exception != nullptr) throw except::group_coro_exception{std::move(exceptions), loc};
    }

private:
    all_state _state{};
    std::vector<std::coroutine_handle<>> _children{}; ///< the children with a coroutine
};

/**
 * the result of a child in the results of `co::all()`
 * @details a reference -> a `cth::ref_wrapper` in a `std::vector`
 */
template<class T>
using all_element_t = std::conditional_t<
    std::is_reference_v<T>,
    cth::ref_wrapper<std::remove_reference_t<T>>,
    T>;

/**
 * awaiter of `co::all()` with awaitables
 * @tparam Awaitables the awaitables, a reference -> the awaiter keeps the reference
 */
template<class... Awaitables>
class all_awaiter : public all_awaiter_base {
public:
    explicit all_awaiter(std::tuple<Awaitables...>&& awaitables) : _awaitables{std::move(awaitables)} {}

    /**
     * starts the children, resumes the caller after the last one
     * @param caller coroutine of the caller, its context and its payload go to the children
     * @return `false` -> every child ended at once, the caller does not suspend
     * @throws what the allocation of a coroutine throws, no child started then
     */
    template<this_coro_compatible Caller>
    bool await_suspend(std::coroutine_handle<Caller> caller) {
        reserve(sizeof...(Awaitables));
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            (add(caller, std::forward<Awaitables>(std::get<Is>(_awaitables)), std::get<Is>(_slots)), ...);
        }(std::index_sequence_for<Awaitables...>{});
        return start(caller);
    }

    /**
     * @return the results in child order, no element for a void child, `void` if every child is void
     * @throws cth::except::group_coro_exception reason: a child failed
     */
    auto await_resume() {
        throw_failures(std::apply(
            [](auto const&... slots) { return std::vector<std::exception_ptr>{slots.exception...}; },
            _slots
        ));

        if constexpr((mta::is_void<awaited_t<Awaitables>> && ...)) return;
        else
            return [this]<size_t... Is>(std::index_sequence<Is...>) {
                return std::tuple_cat(element<Is>()...);
            }(std::index_sequence_for<Awaitables...>{});
    }

private:
    /**
     * @return the result of the child in a tuple, an empty tuple for a void child
     */
    template<size_t I>
    [[nodiscard]] auto element() {
        using result_type = awaited_t<std::tuple_element_t<I, std::tuple<Awaitables...>>>;
        if constexpr(mta::is_void<result_type>) return std::tuple<>{};
        else return std::tuple<result_type>{std::forward<result_type>(*std::get<I>(_slots).result)};
    }

    std::tuple<Awaitables...> _awaitables;
    std::tuple<all_slot<awaited_t<Awaitables>>...> _slots{};
};

/**
 * awaiter of `co::all()` with a range of awaitables
 * @tparam Range of awaitables, a reference -> the awaiter keeps the reference
 */
template<class Range>
class all_range_awaiter : public all_awaiter_base {
    using result_type = awaited_t<std::ranges::range_reference_t<Range>>;

public:
    explicit all_range_awaiter(Range&& range) : _range{std::forward<Range>(range)} {}

    /**
     * starts the children, resumes the caller after the last one
     * @param caller coroutine of the caller, its context and its payload go to the children
     * @return `false` -> every child ended at once, the caller does not suspend
     * @throws what the allocation of a coroutine throws, no child started then
     */
    template<this_coro_compatible Caller>
    bool await_suspend(std::coroutine_handle<Caller> caller) {
        auto const children = static_cast<size_t>(std::ranges::distance(_range));
        _slots.resize(children);
        reserve(children);

        size_t childIdx = 0;
        for(auto& awaitable : _range)
            add(caller, awaitable, _slots[childIdx++]);
        return start(caller);
    }

    /**
     * @return the results in child order, a reference as a `cth::ref_wrapper`, `void` for void children
     * @throws cth::except::group_coro_exception reason: a child failed
     */
    auto await_resume() {
        std::vector<std::exception_ptr> exceptions{};
        exceptions.reserve(_slots.size());
        for(auto const& slot : _slots)
            exceptions.push_back(slot.exception);
        throw_failures(std::move(exceptions));

        if constexpr(mta::is_void<result_type>) return;
        else {
            std::vector<all_element_t<result_type>> results{};
            results.reserve(_slots.size());
            for(auto& slot : _slots)
                results.emplace_back(std::forward<result_type>(*slot.result));
            return results;
        }
    }

private:
    Range _range;
    std::vector<all_slot<result_type>> _slots{};
};
}
