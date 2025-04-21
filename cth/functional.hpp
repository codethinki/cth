#pragma once
#include "types/concepts.hpp"

#include <functional>


namespace cth::fn {

template<auto Fn, class T>
auto to_lambda(T& obj) {
    return [&obj]<class... Args>(Args&&... args) {
        static_assert(cth::type::callable_with<T, Fn, Args...>, "Fn must be callable on T");
        return std::invoke(Fn, obj, std::forward<Args>(args)...);
    };
}

}
