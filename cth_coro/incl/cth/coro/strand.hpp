#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <memory>


namespace cth::co {
/**
 * serial queue on a `scheduler`
 * @details its jobs run one at a time and in post order on the workers of the scheduler
 * @details a coroutine holds the strand between two suspension points, not across a `co_await`
 * @attention the scheduler must outlive the strand
 */
class strand final : public context {
    struct Impl;

public:
    /**
     * constructs the strand on the scheduler
     * @param scheduler to run the jobs on
     */
    explicit strand(scheduler const& scheduler);
    /**
     * destroys the asio strand
     * @pre no coroutine runs or waits on the strand
     */
    ~strand();

    void post(void_func work) const override;

    void await(native_handle, await_func cb) const override;
    void await(chrono::time_point_t time_point, await_func cb) const override;

    [[nodiscard]] context const& detach(std::coroutine_handle<>) const override;
    void release(std::coroutine_handle<>) const override;

private:
    [[nodiscard]] Impl& impl() const { return *_impl; }

    std::unique_ptr<Impl> _impl;

public:
    /**
     * checks if the calling thread runs a job of this strand
     */
    [[nodiscard]] bool owns_thread() const override;

    strand(strand const& other) = delete;
    strand& operator=(strand const& other) = delete;
    /**
     * moves `other` into this strand
     * @attention executors, coroutines and jobs of `other` still refer to `other`
     */
    strand(strand&& other) noexcept;
    /**
     * moves `other` into this strand
     * @attention executors, coroutines and jobs of `other` still refer to `other`
     */
    strand& operator=(strand&& other) noexcept;
};
}
