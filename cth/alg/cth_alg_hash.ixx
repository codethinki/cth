module;

export module cth.alg.hash;
import std;

namespace cth::alg::hash {
    // from: https://stackoverflow.com/a/57595105
    template<typename T, typename... Rest>
    void combine(size_t& seed, T const& v, Rest const&... rest) {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (cth::alg::hash::combine(seed, rest), ...);
    }

} // namespace cth::alg::hash
