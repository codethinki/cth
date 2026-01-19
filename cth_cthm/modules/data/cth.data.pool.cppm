module;

#include <deque>
#include <functional>
#include <memory>
#include <ranges>
#include <unordered_set>
#include <vector>

export module cth.data.pool;
import cth.meta.concepts;

export namespace cth::dt {

struct basic_pool_manipulator {};

/**
 * generic reusable resource pool
 * @tparam T to pool
 * @tparam Manipulator may implement void reset(T&) to reset released instances
 */
template<class T, class Manipulator = basic_pool_manipulator>
class pool {
    using storage_type = std::deque<T>;

public:
    static constexpr bool HAS_RESET = requires(Manipulator m, T& t) { { m.reset(t) } -> mta::is_void; };
    using value_type = T;

    explicit pool(Manipulator manipulator = {}) : _manipulator{std::move(manipulator)} {}

    /**
     * constructs an instance of T in the pool
     * @post capacity increases by one
     * @param args to construct @ref T with
     */
    template<class... CArgs> requires std::constructible_from<T, CArgs...>
    void emplace(CArgs&&... args) {
        _storage.emplace_back(std::forward<CArgs>(args)...);
        _inactive.emplace_back(&_storage.back());
    }

    /**
     * forwards range to storage
     * @post pools capacity increases by size of @ref Rng 
     */
    template<class Rng> requires requires(Rng r, storage_type s) { s.append_range(std::forward<Rng>(r)); }
    void append_range(Rng&& rng) {
        size_t oldSize = _storage.size();

        _storage.append_range(std::forward<Rng>(rng));

        auto pointers = std::views::transform(
            std::ranges::subrange{_storage.begin() + oldSize, _storage.end()},
            [](auto& obj) { return std::addressof(obj); }
        );

        _inactive.append_range(pointers);
    }

    /**
     * acquires a resource from the pool, will not be acquired again until released
     * @pre must not be @ref exhausted()
     * @post resource will not be acquired again until @ref release(T&) is called with this instance
     */
    [[nodiscard]] T& acquire() {
        auto& back = *_inactive.back();
        _inactive.pop_back();
        return back;
    }

    /**
     * @pre @ref t was acquired, not already released and the pool not cleared
     * @post @ref t is reset and can be acquired again
     */
    void release(T& t) {
        auto const ptr = std::addressof(t);

        if constexpr(HAS_RESET)
            _manipulator.reset(t);

        _inactive.push_back(ptr);
    }

    /**
     * clears all acquires and resets acquired objects
     * @pre calling release on a resource which was acquired before clear is UB
     */
    void clear() {
        resetActive();

        _inactive.clear();
        _inactive.append_range(
            _storage | std::views::transform(
                [](auto& element) { return std::addressof(element); }
            )
        );
    }

private:
    void resetActive() {
        if constexpr(HAS_RESET) {
            std::unordered_set inactiveSet{
                std::from_range, _inactive
            };

            for(auto& stored : _storage)
                if(!inactiveSet.contains(&stored))
                    _manipulator.reset(stored);
        }
    }

    Manipulator _manipulator;

    std::vector<T*> _inactive{};
    std::deque<T> _storage{};

public:
    /**
     * total number of instances in pool
     */
    [[nodiscard]] size_t capacity() const noexcept {
        return _storage.size();
    }

    /**
     * remaining instances to acquire without releasing
     */
    [[nodiscard]] size_t remaining() const noexcept {
        return _inactive.size();
    }

    /**
     * true if no more acquire calls are possible
     */
    [[nodiscard]] bool exhausted() const noexcept {
        return _inactive.empty();
    }

};

}
