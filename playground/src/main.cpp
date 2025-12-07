#include <cth/hash/hash_general.hpp>

struct B;

struct A{};

#include <print>
int main() {
    std::println("{}", std::is_base_of_v<B, A>);

}



