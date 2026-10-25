#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <memory>


namespace cth::co {
/**
 * serial queue on a @ref scheduler
 * @details its jobs run one at a time and in post order on the workers of the scheduler,
 *  a coroutine holds the strand between two suspension points, not across a co_await
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
    ~strand();

    void post(void_func work) const override;

    void await(native_handle, await_func cb) const override;
    void await(chrono::time_point_t time_point, await_func cb) const override;

private:
    [[nodiscard]] Impl& impl() const { return *_impl; }

    std::unique_ptr<Impl> _impl;

public:
    [[nodiscard]] bool owns_thread() const override;

    strand(strand const& other) = delete;
    strand& operator=(strand const& other) = delete;
    strand(strand&& other) noexcept;
    strand& operator=(strand&& other) noexcept;
};
}
