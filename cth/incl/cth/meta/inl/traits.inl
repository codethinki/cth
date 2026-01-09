#pragma once
#include "cth/meta/concepts.hpp"

#include <stdexcept>


namespace cth::mta {
template<class T, template<class> class Trait, size_t N>
[[nodiscard]] cval auto iterate_trait() {
    constexpr bool valid = applicable<T, Trait>;

    if constexpr(N == 0) return std::type_identity<T>{};
    else if constexpr(!valid) static_assert(valid, "[Trait] not applicable on [T]");
    else return iterate_trait<Trait<T>, Trait, N - 1>();
}


namespace dev {
    template<class T, template<class> class Trait, size_t Max>
    cval size_t trait_count(size_t n = 0) {
        if constexpr(!applicable<T, Trait>) return n;
        else {
            if(n >= Max) return n;
            return trait_count<iterate_trait_t<T, Trait>, Trait, Max>(n + 1);
        }
    }
}


template<class T, template<class> class Trait, size_t Max>
[[nodiscard]] cval size_t trait_count() { return dev::trait_count<T, Trait, Max>(); }


namespace dev {
    // ReSharper disable once CppInconsistentNaming
    class Trait_must_be_applicable_on_T final : public std::logic_error {
    public:
        Trait_must_be_applicable_on_T() : logic_error{""} {}
    };

    template<class T, auto TCpt, template<class> class Trait, size_t MaxDepth>
    cval size_t cpt_count(size_t n = 0) {
        if(n >= MaxDepth) return n;
        if constexpr(!satisfies<T, TCpt>) return n;
        else if constexpr(!applicable<T, Trait>) throw Trait_must_be_applicable_on_T{};
        else {
            using U = iterate_trait_t<T, Trait>;
            return cpt_count<U, TCpt, Trait, MaxDepth>(n + 1);
        }
    }
}


template<class T, auto TCpt, template<class> class Trait, size_t MaxDepth>
[[nodiscard]] cval size_t cpt_count() { return dev::cpt_count<T, TCpt, Trait, MaxDepth>(); }

}
