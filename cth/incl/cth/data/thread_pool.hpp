#pragma once
#include "cth/data/hash_map.hpp"
#include "cth/data/pool.hpp"

#include <cstddef>
#include <mutex>
#include <thread>

namespace cth::dt {

/**
 * thread safe `pool` that binds an acquired resource to a thread id
 * @details a thread acquires a resource once and keeps it across calls until it releases it, so every
 *  thread owns at most one resource at a time
 * @tparam T to pool
 * @tparam Manipulator may implement `void reset(T&)` to reset released instances
 */
template<class T, class Manipulator = basic_pool_manipulator>
class thread_pool {
    using pool_t = pool<T, Manipulator>;

public:
    using value_type = T;
    static constexpr bool HAS_RESET = pool_t::HAS_RESET;

    explicit thread_pool(Manipulator manipulator = {}) : _pool{std::move(manipulator)} {}

    /**
     * constructs an instance of T in the pool
     * @post capacity increases by one
     * @param args to construct `T` with
     */
    template<class... CArgs> requires std::constructible_from<T, CArgs...>
    void emplace(CArgs&&... args) {
        std::lock_guard lock{_mtx};
        _pool.emplace(std::forward<CArgs>(args)...);
    }

    /**
     * forwards range to storage
     * @post capacity increases by the size of `rng`
     */
    template<class Rng>
    void append_range(Rng&& rng) {
        std::lock_guard lock{_mtx};
        _pool.append_range(std::forward<Rng>(rng));
    }

    /**
     * the resource bound to `id`, acquiring a fresh one from the pool the first time `id` asks
     * @pre the pool must not be `exhausted()` when `id` has no resource yet
     * @post `id` owns the returned resource until `release()`
     */
    [[nodiscard]] T& acquire(std::thread::id id = std::this_thread::get_id()) {
        std::lock_guard lock{_mtx};

        auto const it = _bound.find(id);
        if(it != _bound.end())
            return *it->second;

        auto& resource = _pool.acquire();
        _bound.emplace(id, &resource);
        return resource;
    }

    /**
     * releases the resource bound to `id` back to the pool
     * @pre `id` currently owns a resource
     * @post the resource is reset (if supported) and can be acquired again
     */
    void release(std::thread::id id = std::this_thread::get_id()) {
        std::lock_guard lock{_mtx};

        auto const it = _bound.find(id);
        CTH_CRITICAL(it == _bound.end(), "thread holds no resource to release") {}

        _pool.release(*it->second);
        _bound.erase(it);
    }

    /**
     * releases every bound resource back to the pool
     * @pre no concurrent `acquire` / `release`
     */
    void clear() {
        std::lock_guard lock{_mtx};
        _pool.clear();
        _bound.clear();
    }

private:
    mutable std::mutex _mtx;
    pool_t _pool;
    flat_hash_map<std::thread::id, T*> _bound{};

public:
    /** total number of instances in the pool */
    [[nodiscard]] size_t capacity() const {
        std::lock_guard lock{_mtx};
        return _pool.capacity();
    }

    /** remaining instances to acquire without releasing */
    [[nodiscard]] size_t remaining() const {
        std::lock_guard lock{_mtx};
        return _pool.remaining();
    }

    /** `true` if no more acquire calls are possible */
    [[nodiscard]] bool exhausted() const {
        std::lock_guard lock{_mtx};
        return _pool.exhausted();
    }

    /** number of threads currently holding a resource */
    [[nodiscard]] size_t bound() const {
        std::lock_guard lock{_mtx};
        return _bound.size();
    }
};

}
