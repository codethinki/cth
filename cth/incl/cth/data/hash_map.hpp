#pragma once
#include <gtl/phmap.hpp>

namespace cth::dt {
/**
 * flat hash map
 * @details no pointer stability: a rehash moves the elements
 */
template<class Key, class T, class... Args>
using flat_hash_map = gtl::flat_hash_map<Key, T, Args...>;

/**
 * flat hash set
 * @details no pointer stability: a rehash moves the elements
 */
template<class Key, class... Args>
using flat_hash_set = gtl::flat_hash_set<Key, Args...>;
}
