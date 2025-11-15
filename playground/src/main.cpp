#include <cth/hash/hash_general.hpp>

#include <print>
int main() {
    std::string asdf = "asdf";
    std::string jklö = "jklö";

    std::println("hash: (l, r): {}", cth::hash::combine(asdf, jklö));
    std::println("hash: (r, l): {}", cth::hash::combine(jklö, asdf));

}



