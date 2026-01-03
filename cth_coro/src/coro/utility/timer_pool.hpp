#pragma once
#include <cth/os/osdef.hpp>
#include "boost.hpp"
#include "boost/asio/post.hpp"
#include "cth/coro/utility/fwd.hpp"
#include <boost/asio/io_context.hpp>


#ifdef CTH_FS_WINDOWS
#include "native_handle_helpers.hpp"



#include <cth/data/pool.hpp>
#include <cth/win/coro/timer.hpp>

namespace cth::co {
namespace dev {


    /**
     * win timer with boost handle integration
     */
    class adapted_timer {

    public:
        adapted_timer(bas::io_context& ctx) : _timer{},
            _handler{
                ctx,
                duplicate_awaitable_native_handle(_timer.native_handle())
            } {}

        [[nodiscard]] native_handle_handler_t& set(time_point_t time_point) {
            _timer.set(time_point);
            return _handler;
        }

    private:
        win::co::timer _timer;
        native_handle_handler_t _handler;
    };

}



class timer_pool {

public:
    using void_func = std::move_only_function<void()>;
    using timer_t = dev::adapted_timer;

    timer_pool(bas::io_context& ctx) : _ctx{ctx} {}

    void set(time_point_t time_point, void_func callback) {
        auto& timer = new_timer();
        auto& handler = timer.set(time_point);

        handler.async_wait(
            [&handler, &timer, this, cb = std::move(callback)](boost::system::error_code const& ec) mutable {
                BOOST_EC_STABLE_THROW(ec, "async wait for handle [{}] failed", handler.native_handle())

                release(timer);

                cb();
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


#else
#include <boost/asio/steady_timer.hpp>


namespace cth::co {

class timer_pool {
public:
    using void_func = std::move_only_function<void()>;

    timer_pool(boost::asio::io_context& ctx) : _ctx{ctx} {}

    void set(cth::co::time_point_t time_point, void_func callback) const {
        auto timer = std::make_unique<bas::steady_timer>(_ctx, time_point);

        timer->async_wait(
            [this, time_point, t = std::move(timer), cb = std::move(callback)](
            boost::system::error_code const& ec
        ) mutable {
                BOOST_EC_STABLE_THROW(
                    ec,
                    "async wait for time_point [{}] failed",
                    time_point.time_since_epoch()
                )

                cb();
            }
        );
    }

private:
    bas::io_context& _ctx;
};
}


#endif
