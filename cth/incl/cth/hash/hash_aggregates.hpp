#pragma once
#include "hash_general.hpp"

#include <boost/pfr.hpp>

#include <functional>


namespace cth::hash {
template<type::aggregate T>
cxpr size_t hash_aggregate(T const& t) {
    return std::apply(
        []<class... Args>(Args&&... args) { return hash::combine(std::forward<Args>(args)...); },
        boost::pfr::structure_tie(t)
    );
}
}


#define CTH_HASH_AGGREGATE(type)\
template<>\
struct std::hash<type> {\
    cxpr size_t operator()(type const& x) const noexcept {\
        return cth::hash::hash_aggregate(x);\
    }\
};
