#pragma once
#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/coro.hpp>

namespace cth::co {
/**
 * base for this_coro participants specifying the payload
 * @details inherit from this base to mark your type this coro compatible.
 *  most likely has to implement an inject(payload auto const& p)
 * @tparam Pyld payload carried through the coroutine
 */
template<payload Pyld>
struct this_coro_base : dev::this_coro_base_type {
    using payload_type = Pyld;
};

template<class T>
using payload_t = std::remove_cvref_t<T>::payload_type;


}

namespace cth::co::this_coro {
template<class T, class Pyld>
concept awaitable_injectable_with = this_coro_compatible<co::awaiter_t<T>>
    && pyld_compatible_with<payload_t<co::awaiter_t<T>>, Pyld>;

template<class Promise, class Pyld>
concept promise_injectable_with =
    this_coro_compatible<Promise>
    && pyld_compatible_with<payload_t<Promise>, Pyld>;
}
