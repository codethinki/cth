#pragma once
#include "cth/coro/utility/concepts.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <coroutine>

namespace cth::co::this_coro {
struct executor_tag {};
struct scheduler_tag {};

inline constexpr executor_tag executor{};
inline constexpr scheduler_tag scheduler{};



namespace dev {
    template<class T>
    class data_awaiter {
    public:
        data_awaiter(T type) : _type(type) {}

        bool await_ready() const noexcept { return true; }
        void await_suspend(std::coroutine_handle<>) const noexcept {}
        T await_resume() const noexcept { return _type; }

    private:
        T _type;
    };
}

struct payload {
    co::executor get(executor_tag) const;
    co::scheduler& get(scheduler_tag) const { return *_scheduler; }

    template<tag T>
    auto get() { return this->get(T{}); }

    co::scheduler& scheduler() const { return *_scheduler; }

    template<tag T>
    auto await() { return dev::data_awaiter{payload::get<T>()}; }

    payload(co::scheduler& scheduler) : _scheduler{&scheduler} {}

private:
    co::scheduler* _scheduler;
};

}
