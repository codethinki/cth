#pragma once
#include "cth/coro/exception.hpp"
#include "cth/coro/tasks/dev/all_child_task.hpp"
#include "cth/coro/tasks/promises/raw_promise.hpp"
#include "cth/coro/this_coro/this_coro_base.hpp"
#include "cth/coro/utility/concepts.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/coro/utility.hpp>
#include <cth/meta/concepts.hpp>
#include <cth/meta/coro.hpp>
#include <cth/ptr/ref_wrapper.hpp>

#include <algorithm>
#include <array>
#include <coroutine>
#include <cstddef>
#include <exception>
#include <memory>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>


namespace cth::co::dev {
/**
 * deleter of `all_child_ptr`: destroys the coroutine of the child
 */
struct all_child_deleter {
    std::coroutine_handle<> handle = nullptr; ///< coroutine of the child

    void operator()(raw_promise_base const*) const { handle.destroy(); }
};

/**
 * owner of the coroutine of a child of `co::all()`, points at the promise of the child
 * @details `get_deleter().handle` is the coroutine
 * @details destroys the coroutine at its destruction: the child did not start or it ended
 * @tparam T result type of the child
 */
template<class T>
using all_child_ptr = std::unique_ptr<raw_promise<T>, all_child_deleter>;

/**
 * starts the children of `co::all()`
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
     * a ready awaiter: stores its result in the slot at once, else creates the child that awaits the awaitable
     * @details a reference stays a reference, the child owns a temporary awaiter or task
     * @details the child starts in `start()`
     * @tparam T result type of the awaitable
     * @param caller coroutine of the caller, the child gets its context and its payload
     * @param awaitable to await, a reference lives until the end of the `co_await`
     * @param[out] slot gets the result or the exception of a ready awaiter
     * @return the child, `nullptr` -> ready, the slot holds the result or the exception
     * @throws what the allocation of the coroutine throws
     */
    template<class T, class Caller, awaitable Awaitable>
    [[nodiscard]] all_child_ptr<T> add(
        std::coroutine_handle<Caller> caller,
        Awaitable&& awaitable,
        raw_promise<T>& slot
    ) {
        if constexpr(!std::is_reference_v<Awaitable> && task<Awaitable>)
            return addChild<T, Awaitable>(caller, std::forward<Awaitable>(awaitable));
        else {
            using extracted_type = decltype(co::extract_awaiter(std::forward<Awaitable>(awaitable)));
            using awaiter_type = std::conditional_t<
                std::is_reference_v<Awaitable>,
                extracted_type,
                std::remove_cvref_t<extracted_type>>;
            awaiter_type awaiter = co::extract_awaiter(std::forward<Awaitable>(awaitable));

            try {
                if(awaiter.await_ready()) {
                    if constexpr(mta::is_void<T>) {
                        awaiter.await_resume();
                        slot.return_void();
                    } else
                        slot.return_value(awaiter.await_resume());
                    return nullptr;
                }
            }
            catch(...) {
                slot.unhandled_exception();
                return nullptr;
            }
            return addChild<T, awaiter_type>(caller, std::forward<awaiter_type>(awaiter));
        }
    }

    /**
     * starts the children in turn on the calling thread, each runs until it suspends
     * @param caller coroutine of the caller
     * @param handles coroutines of the children, `nullptr` -> no coroutine
     * @return `false` -> every child ended, the caller does not suspend
     */
    template<this_coro_compatible Caller, std::ranges::forward_range Handles>
    [[nodiscard]] bool start(std::coroutine_handle<Caller> caller, Handles&& handles) {
        _state.callerContext = &caller.promise().context();

        auto const coroutines = std::ranges::distance(handles) - std::ranges::count(handles, nullptr);
        // one more for the start: no child resumes the caller before every child started
        _state.count.store(static_cast<size_t>(coroutines) + 1, std::memory_order_relaxed);

        for(auto const handle : handles)
            if(handle) handle.resume();

        // the start is the last: the children ended
        return _state.count.fetch_sub(1, std::memory_order_acq_rel) != 1;
    }

    /**
     * throws if a child failed
     * @param children count of the children
     * @param exceptions child index and exception of each child that can have failed, `nullptr` -> none
     * @throws cth::except::group_coro_exception reason: a child failed, one exception per child
     */
    template<std::ranges::forward_range Exceptions>
    static void throw_failures(size_t children, Exceptions&& exceptions) {
        auto const failed = [](auto const& child) { return std::get<1>(child) != nullptr; };
        if(std::ranges::none_of(exceptions, failed)) return;

        std::vector<std::exception_ptr> perChild(children);
        for(auto const& [childIdx, exception] : exceptions)
            perChild[childIdx] = exception;
        throw except::group_coro_exception{std::move(perChild)};
    }

private:
    /**
     * creates the child that awaits the awaitable, it awaits the caller as its continuation
     * @return the child
     */
    template<class T, class Awaitable, class Caller>
    [[nodiscard]] all_child_ptr<T> addChild(std::coroutine_handle<Caller> caller, Awaitable&& awaitable) {
        auto const& promise = caller.promise();
        auto const child = all_child<T, payload_t<Caller>, Awaitable>(
            _state,
            promise.context(),
            promise.payload(),
            std::forward<Awaitable>(awaitable)
        );
        child.handle.promise().continuation = caller;
        return all_child_ptr<T>{&child.handle.promise(), all_child_deleter{child.handle}};
    }

    all_state _state{};
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
    static constexpr size_t CHILDREN = sizeof...(Awaitables);

public:
    explicit all_awaiter(std::tuple<Awaitables...>&& awaitables) : _awaitables{std::move(awaitables)} {}

    /**
     * creates the children, then starts them, see `start()`
     * @throws what the allocation of a coroutine throws, no child started then
     */
    template<this_coro_compatible Caller>
    bool await_suspend(std::coroutine_handle<Caller> caller) {
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            ((std::get<Is>(_coroutines) = add(caller, std::get<Is>(_awaitables), std::get<Is>(_slots))), ...);
        }(std::index_sequence_for<Awaitables...>{});

        auto const handles = std::apply(
            [](auto const&... coroutines) {
                return std::array<std::coroutine_handle<>, CHILDREN>{coroutines.get_deleter().handle...};
            },
            _coroutines
        );
        return start(caller, handles);
    }

    /**
     * @return the results, see `co::all()`
     * @throws cth::except::group_coro_exception reason: a child failed
     */
    auto await_resume() {
        [this]<size_t... Is>(std::index_sequence<Is...>) {
            throw_failures(
                CHILDREN,
                std::array<std::pair<size_t, std::exception_ptr>, CHILDREN>{
                    std::pair{Is, result<Is>().exception}...
                }
            );
        }(std::index_sequence_for<Awaitables...>{});

        if constexpr((mta::is_void<awaited_t<Awaitables>> && ...)) return;
        else
            return [this]<size_t... Is>(std::index_sequence<Is...>) {
                return std::tuple_cat(element<Is>()...);
            }(std::index_sequence_for<Awaitables...>{});
    }

private:
    /**
     * @return the slot or the promise of the child
     */
    template<size_t I>
    [[nodiscard]] auto& result() {
        auto& coroutine = std::get<I>(_coroutines);
        return coroutine != nullptr ? *coroutine : std::get<I>(_slots);
    }

    /**
     * @return the result of the child in a tuple, an empty tuple for a void child
     */
    template<size_t I>
    [[nodiscard]] auto element() {
        using result_type = awaited_t<std::tuple_element_t<I, std::tuple<Awaitables...>>>;
        if constexpr(mta::is_void<result_type>) return std::tuple<>{};
        else return std::tuple<result_type>{std::forward<result_type>(*result<I>().result)};
    }

    std::tuple<Awaitables...> _awaitables;
    std::tuple<raw_promise<awaited_t<Awaitables>>...> _slots{}; ///< the results of the ready awaiters
    // after the awaitables: destroyed first, a child can refer to an awaitable
    /** per child: its coroutine, `nullptr` -> ready, the result is in its slot */
    std::tuple<all_child_ptr<awaited_t<Awaitables>>...> _coroutines{};
};

/**
 * awaiter of `co::all()` with a range of awaitables
 * @tparam Range of awaitables, a reference -> the awaiter keeps the reference
 */
template<class Range>
class all_range_awaiter : public all_awaiter_base {
    using result_type = awaited_t<std::ranges::range_reference_t<Range>>;

    /**
     * child with a coroutine, a result or an exception
     */
    struct child_entry {
        size_t childIdx; ///< in the order of the range
        raw_promise<result_type> slot{}; ///< the result or the exception of a ready awaiter
        /** `nullptr` -> ready, the result is in `child_entry::slot` */
        all_child_ptr<result_type> coroutine{};

        /**
         * @return the slot or the promise of the child
         */
        [[nodiscard]] raw_promise<result_type>& result() { return coroutine != nullptr ? *coroutine : slot; }
    };

public:
    explicit all_range_awaiter(Range&& range) : _range{std::forward<Range>(range)} {}

    /**
     * creates the children, then starts them
     * @details the details of `all_awaiter::await_suspend()`
     * @details void results: a ready child without an exception gets no entry
     * @details at most one allocation for the entries: the first entry reserves room for every child after it
     */
    template<this_coro_compatible Caller>
    bool await_suspend(std::coroutine_handle<Caller> caller) {
        _children = static_cast<size_t>(std::ranges::distance(_range));

        size_t childIdx = 0;
        for(auto&& awaitable : _range) {
            child_entry entry{.childIdx = childIdx++};
            entry.coroutine = add(caller, std::forward<decltype(awaitable)>(awaitable), entry.slot);

            // nothing to give and nothing to throw
            if constexpr(mta::is_void<result_type>)
                if(entry.coroutine == nullptr && entry.slot.exception == nullptr) continue;

            if(_entries.empty()) _entries.reserve(_children - entry.childIdx);
            _entries.push_back(std::move(entry));
        }

        auto const handle = [](child_entry const& entry) { return entry.coroutine.get_deleter().handle; };
        return start(caller, _entries | std::views::transform(handle));
    }

    /**
     * @return the results, see `co::all()`
     * @throws cth::except::group_coro_exception reason: a child failed
     */
    auto await_resume() {
        auto const exception = [](child_entry& entry) {
            return std::pair{entry.childIdx, entry.result().exception};
        };
        throw_failures(_children, _entries | std::views::transform(exception));

        if constexpr(mta::is_void<result_type>) return;
        else {
            // every child has an entry, in child order
            std::vector<all_element_t<result_type>> results{};
            results.reserve(_entries.size());
            for(auto& entry : _entries)
                results.emplace_back(std::forward<result_type>(*entry.result().result));
            return results;
        }
    }

private:
    Range _range;
    size_t _children = 0; ///< count of the children of the range

    // after the range: destroyed first, a child can refer to an element of the range
    /**
     * the children with a coroutine, a result or an exception, in child order
     * @details void results: no entry for a ready child without an exception
     */
    std::vector<child_entry> _entries{};
};
}
