#pragma once
#include "this_coro.hpp"

#include <coroutine>
#include <exception>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace cth::co {
// Forward declaration of the main task class
template<typename T>
class [[nodiscard]] executor_task;

namespace dev {
    // A storage utility that mimics std::optional's interface for both T and void.
    template<typename T>
    struct result_storage {
        constexpr result_storage() = default;

        template<typename... Args>
        constexpr void emplace(Args&&... args) { _value.emplace(std::forward<Args>(args)...); }

        constexpr void swap(result_storage& other) noexcept { _value.swap(other._value); }

        template<typename Self>
        constexpr auto&& operator*(this Self&& self) { return *std::forward<Self>(self)._value; }

    private:
        std::optional<T> _value;

    public:
        [[nodiscard]] constexpr bool has_value() const noexcept { return _value.has_value(); }
        [[nodiscard]] constexpr operator bool() const noexcept { return has_value(); }

    };

    // ReSharper disable CppMemberFunctionMayBeStatic
    template<>
    struct result_storage<void> {
        constexpr void emplace() noexcept {}
        constexpr void swap(result_storage&) noexcept {}
        constexpr void operator*() const noexcept {}
    };
    // ReSharper restore CppMemberFunctionMayBeStatic


    struct final_awaiter {
        // ReSharper disable once CppMemberFunctionMayBeStatic
        bool await_ready() noexcept { return false; }
        template<typename Promise>
        std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise> h) noexcept {
            if(auto continuation = h.promise().continuation) return continuation;
            return std::noop_coroutine();
        }
        // ReSharper disable once CppMemberFunctionMayBeStatic
        void await_resume() noexcept {}
    };

    template<typename T>
    struct task_awaiter : executor_awaiter_base {
        static constexpr bool VOID = std::is_void_v<T>;

        using promise_type = typename executor_task<T>::promise_type;
        std::coroutine_handle<promise_type> handle;

        [[nodiscard]] bool await_ready() const noexcept { return !handle || handle.done(); }

        std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaiting_coroutine) noexcept {
            inject_executor(awaiting_coroutine);
            handle.promise().continuation = awaiting_coroutine;
            return handle;
        }

        T await_resume() {
            if(handle.promise().exception)
                std::rethrow_exception(handle.promise().exception);

            if constexpr(VOID) return;
            else return std::move(*handle.promise().result);
        }
    };

    // Base promise type with common functionality
    template<typename T>
    struct promise_base : executor_promise_base {
        using value_type = T;
        using executor_promise_base::executor_promise_base;

        dev::result_storage<value_type> result;
        std::exception_ptr exception;
        std::coroutine_handle<> continuation = nullptr;

        std::suspend_always initial_suspend() noexcept { return {}; }

        auto final_suspend() noexcept { return dev::final_awaiter{}; }

        void unhandled_exception() { exception = std::current_exception(); }

        // Non-void: return_value
        void return_value(T&& value) noexcept { result.emplace(std::move(value)); }

        void return_value(T const& value) noexcept requires std::copy_constructible<T> { result.emplace(value); }
    };

    // Specialization for void
    template<>
    struct promise_base<void> : executor_promise_base {
        using value_type = void;
        using executor_promise_base::executor_promise_base;

        dev::result_storage<void> result;
        std::exception_ptr exception;
        std::coroutine_handle<> continuation = nullptr;

        std::suspend_always initial_suspend() noexcept { return {}; }

        auto final_suspend() noexcept { return dev::final_awaiter{}; }

        void unhandled_exception() { exception = std::current_exception(); }

        // Void: return_void
        void return_void() noexcept { result.emplace(); }
    };
}


template<typename T>
class [[nodiscard]] executor_task {
public:
    struct promise_type : dev::promise_base<T> {
        using dev::promise_base<T>::promise_base;

        executor_task<T> get_return_object() noexcept {
            return executor_task<T>{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
    };

    ~executor_task() { optDestroy(); }

    auto operator co_await() const & noexcept { return dev::task_awaiter<T>{_handle}; }
    auto operator co_await() && noexcept { return dev::task_awaiter<T>{std::exchange(_handle, nullptr)}; }

    void resume() { if(!done()) _handle.resume(); }

private:
    void optDestroy() { if(_handle) _handle.destroy(); }

    std::coroutine_handle<promise_type> _handle;
    explicit executor_task(std::coroutine_handle<promise_type> handle) : _handle(handle) {}

public:
    [[nodiscard]] bool done() const noexcept { return !_handle || _handle.done(); }


    executor_task(executor_task const&) = delete;
    executor_task& operator=(executor_task const&) = delete;

    executor_task(executor_task&& other) noexcept : _handle{std::exchange(other._handle, nullptr)} {}
    executor_task& operator=(executor_task&& other) noexcept {
        auto& self = *this;

        if(this == &other) return self;

        optDestroy();
        _handle = std::exchange(other._handle, nullptr);

        return self;
    }

};

using executor_void_task = executor_task<void>;

}
