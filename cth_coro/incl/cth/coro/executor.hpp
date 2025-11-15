#pragma once
#include "cth/coro/scheduler.hpp"


namespace cth::co {
template<class T, class Task>
constexpr executor_task<T> inject_executor(executor, Task);
}

namespace cth::co {
class executor {
public:
    explicit executor(scheduler& sched) noexcept : _sched(&sched) {}
    ~executor() = default;

    void post(executor_void_task task) const { _sched->post(std::move(task)); }

    template<class Task>
    void post(Task task) const {
        this->post(
            [](Task task) -> executor_void_task {
                co_await task;
                co_return;
            }(std::move(task))
        );
    }

    template<class T, class Task>
    executor_task<T> spawn(Task task) {
        co_await _sched->transfer_exec();
        co_return co_await inject_executor<T>(*this, std::move(task));
    }

    [[nodiscard]] scheduler::ctx_switch_awaiter transfer_exec() const { return _sched->transfer_exec(); }

    executor(executor const& other) = default;
    executor(executor&& other) noexcept = default;
    executor& operator=(executor const& other) = default;
    executor& operator=(executor&& other) noexcept = default;

private:
    scheduler* _sched;
};


}

namespace cth::co {
template<class T, class Task>
constexpr executor_task<T> inject_executor([[maybe_unused]] executor exec, Task task) {
    if constexpr(std::is_void_v<T>) {
        co_await task;
        co_return;
    } else co_return co_await task;
}
}
