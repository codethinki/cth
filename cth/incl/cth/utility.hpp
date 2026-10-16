#pragma once
#include <memory>
#include <type_traits>
#include <utility>

namespace cth {
/**
 * @brief destroys the object and move constructs it from other in place
 * @details move assignment for types where `= default` is wrong, e.g. types that own a resource:
 *  `T& operator=(T&& other) noexcept { return cth::reconstruct_at(*this, std::move(other)); }`
 * @param object to replace, its destructor runs first
 * @param other object to move from
 * @return object
 * @pre object is not a base class subobject or a [[no_unique_address]] member
 * @pre object does not own other
 * @note &object == &other -> no op
 */
template<class T> requires(
    std::is_nothrow_move_constructible_v<T> && std::is_nothrow_destructible_v<T> && !std::is_polymorphic_v<T>
)
constexpr T& reconstruct_at(T& object, std::type_identity_t<T>&& other) noexcept {
    if(std::addressof(object) == std::addressof(other)) return object;

    std::destroy_at(std::addressof(object));
    return *std::construct_at(std::addressof(object), std::move(other));
}
}
