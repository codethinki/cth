#pragma once
#include "cth/coro/exception.hpp"

#include <cth/io/log.hpp>

#include <coroutine>
#include <system_error>


namespace cth::co {
/**
 * base of awaiters that wait with @ref context::await(), a failed wait throws in the caller
 * @attention a derived await_resume() calls the one of this base first, else a failed wait goes unnoticed
 */
class wait_awaiter_base {
public:
    /**
     * throws if the wait failed
     * @throws cth::except::coro_exception reason: the wait failed
     */
    void await_resume() const {
        CTH_STABLE_THROW_T(cth::except::coro_exception, _error, "wait failed, got {}", _error.message()) {}
    }

protected:
    /**
     * gives the callback for @ref context::await(), it stores the error and resumes the coroutine
     * @param h coroutine to resume
     * @return callback for @ref context::await()
     */
    [[nodiscard]] auto resume(std::coroutine_handle<> h) {
        return [this, h](std::error_code ec) {
            _error = ec;
            h.resume();
        };
    }

private:
    std::error_code _error{};
};
}
