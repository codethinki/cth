#pragma once
#include <concepts>
#include <type_traits>

namespace cth::co {
class context;
class scheduler;
class simple_counting_scope;
class strand;
class executor;
}

namespace cth::co::this_coro {
/**
 * base of the `this_coro` tags
 * @details a `this_coro` coroutine calls an awaited tag with its context and its payload, the tag gives an awaitable
 */
struct [[nodiscard]] tag_base {};

/**
 * `true` if `Tag` derives `tag_base`
 */
template<class Tag>
concept tag = std::is_base_of_v<tag_base, std::remove_cvref_t<Tag>>;

struct executor_tag;
struct context_tag;

struct wait_tag;

}

namespace cth::co::this_coro {
struct payload_base;

/**
 * payload of tasks without a custom payload
 */
using default_payload = payload_base;

}

namespace cth::co {
/**
 * `true` if `T` can be a payload
 * @details a payload must be copy constructible
 */
template<class T>
concept payload = std::copy_constructible<std::remove_cvref_t<T>>;

namespace dev {
    /**
     * dev only, use `this_coro_base`
     * @details `this_coro_compatible` detects it
     */
    struct this_coro_base_type {};

    /**
     * dev only, use `raw_promise`
     * @details base of every `raw_promise`, defined in raw_promise.hpp
     */
    struct raw_promise_base;
}

template<payload Pyld = this_coro::default_payload>
struct this_coro_base;

/**
 * `true` if `T` is a `this_coro` promise
 * @details `this_coro` promises derive `this_coro_base`
 */
template<class T>
concept this_coro_compatible = std::is_base_of_v<dev::this_coro_base_type, std::remove_cvref_t<T>>;

/**
 * `true` if `T` derives `dev::raw_promise_base`: it has the exception and the continuation
 */
template<class T>
concept raw_promise_type = std::is_base_of_v<dev::raw_promise_base, std::remove_cvref_t<T>>;

/**
 * `true` if `From` and `To` are payloads and `From` converts to `To`
 */
template<class From, class To>
concept pyld_injectable_to = payload<From> && payload<To> && std::convertible_to<From, To>;

/**
 * `true` if `Pyld` and `To` are payloads and `Pyld` converts to `To`
 */
template<class To, class Pyld>
concept pyld_compatible_with = payload<Pyld> && payload<To> && pyld_injectable_to<Pyld, To>;



}



namespace cth::co {
template<class T>
class capture_task;

template<class T>
class sync_task;


template<class T, payload Pyld = this_coro::default_payload>
class executor_task;

template<class T, payload Pyld = this_coro::default_payload>
class spawn_task;
}



namespace cth::except {
class coro_exception;
}

namespace cth::co {
struct schedule_awaiter;
struct time_point_awaiter;

template<payload Pyld = this_coro::default_payload>
struct this_coro_promise_base;

/**
 * base of awaiters that resume the caller on its context themselves
 * @details `this_coro_promise_base` awaits them as they are
 */
struct this_coro_awaiter_base {};

struct sync_promise_base;

}
