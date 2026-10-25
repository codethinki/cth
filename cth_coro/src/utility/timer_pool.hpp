#pragma once
#include "native_handle_helpers.hpp"

#include "cth/coro/utility/fwd.hpp"

#include <cth/data/pool.hpp>
#include <cth/os/timer.hpp>

#include <asio/io_context.hpp>

#include <functional>
#include <mutex>
#include <system_error>


namespace cth::co {
namespace dev {

    /**
     * os timer with boost handle integration
     */
    class adapted_timer {

    public:
        adapted_timer(bas::io_context& ctx) :
            _timer{}, _handler{ctx, os::duplicate_handle(_timer.native_handle())} {}

        [[nodiscard]] native_handle_handler_t& set(chrono::time_point_t time_point) {
            _timer.set(time_point);
            return _handler;
        }

    private:
        cth::os::timer _timer;
        native_handle_handler_t _handler;
    };

}


class timer_pool {

public:
    using await_func = std::move_only_function<void(std::error_code)>;
    using timer_t = dev::adapted_timer;

    timer_pool(bas::io_context& ctx) : _ctx{ctx} {}

    /**
     * calls the callback once the time point is reached
     * @details releases the timer before the callback, the callback gets the error code of the wait
     * @param time_point to wait for
     * @param callback to call with the error code
     */
    void set(chrono::time_point_t time_point, await_func callback) {
        auto& timer = new_timer();
        auto& handler = timer.set(time_point);

        async_wait_signaled(
            handler,
            [&timer, this, cb = std::move(callback)](std::error_code const& ec) mutable {
                release(timer);
                cb(ec);
            }
        );
    }

private:
    void release(timer_t& timer) {
        std::scoped_lock _{_poolMtx};
        _pool.release(timer);
    }
    timer_t& new_timer() {
        std::scoped_lock _{_poolMtx};

        if(_pool.exhausted())
            _pool.emplace(_ctx);

        return _pool.acquire();
    }

    bas::io_context& _ctx;

    std::mutex _poolMtx;
    dt::pool<dev::adapted_timer> _pool{};
};

}
