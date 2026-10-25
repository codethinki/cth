#pragma once
#include "cth/coro/utility/fwd.hpp"

#include <type_traits>

namespace cth::co {
/**
 * base of this_coro promises, specifies the payload
 * @details @ref this_coro_compatible detects it
 * @tparam Pyld payload of the coroutine
 */
template<payload Pyld>
struct this_coro_base : dev::this_coro_base_type {
    using payload_type = Pyld;
};

/**
 * payload type of a this_coro promise
 */
template<class T>
using payload_t = std::remove_cvref_t<T>::payload_type;


}

namespace cth::co::this_coro {
/**
 * true if Promise is a this_coro promise and Pyld converts to its payload
 */
template<class Promise, class Pyld>
concept promise_injectable_with =
    this_coro_compatible<Promise>
    && pyld_compatible_with<payload_t<Promise>, Pyld>;
}
