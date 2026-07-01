#pragma once
#include <concepts>
#include <type_traits>

namespace cth::co {
class scheduler;
class executor;
}

namespace cth::co::this_coro {
/**
 * new this_coro tags must inherit from this
 */
struct [[nodiscard]] tag_base {};

template<class Tag>
concept tag = std::is_base_of_v<tag_base, std::remove_cvref_t<Tag>>;

struct executor_tag;
struct scheduler_tag;

struct wait_tag;

}

namespace cth::co::this_coro {


struct scheduler_payload_base;
template<class Pyld>
concept scheduler_payload = std::is_base_of_v<scheduler_payload_base, std::remove_cvref_t<Pyld>>;

using default_payload = scheduler_payload_base;

}

namespace cth::co {
/**
 * true if `T` is a potential payload type
 * @details must be copy constructible
 */
template<class T>
concept payload = std::copy_constructible<std::remove_cvref_t<T>>;

namespace dev {
    /**
     * dev only, use @ref this_coro_base
     * @details base to detect this_coro support
     */
    struct this_coro_base_type {};
}

template<payload Pyld = this_coro::default_payload>
struct this_coro_base;

/**
 * true if @ref T opts into the this_coro payload machinery by deriving @ref this_coro_base
 */
template<class T>
concept this_coro_compatible = std::is_base_of_v<dev::this_coro_base_type, std::remove_cvref_t<T>>;

/**
 * true if `From` is convertible to `to`
 */
template<class From, class To>
concept pyld_injectable_to = payload<From> && payload<To> && std::convertible_to<From, To>;

/**
 * true if `To` is convertible to `Pyld`
 */
template<class To, class Pyld>
concept pyld_compatible_with = payload<Pyld> && payload<To> && pyld_injectable_to<Pyld, To>;



}



namespace cth::co {
template<class T, payload Pyld = this_coro::default_payload>
class executor_task;

template<class T, payload Pyld = this_coro::default_payload>
class scheduled_task;

template<class T, payload Pyld = this_coro::default_payload>
class sync_executor_task;


template<class T>
class capture_task;
}



namespace cth::except {
class coro_exception;
}

namespace cth::co {
template<this_coro::scheduler_payload Pyld = this_coro::default_payload>
struct schedule_awaiter;
struct wait_awaiter;

template<payload Pyld = this_coro::default_payload>
struct this_coro_promise_awaiter_base;
template<payload Pyld = this_coro::default_payload>
struct this_coro_promise_base;

struct capture_awaiter_base;
struct sync_promise_base;

}
