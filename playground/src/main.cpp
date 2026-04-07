#include <iostream>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <vector>
#include <numeric>
#include <print>

//
//int main() {
//#ifdef  __cpp_reflection
//    std::println("asdf");
//#endif
//    auto x = [
//            y = 1
//        ]() {
//        int x = 10;
//    };
//
//#ifdef _LIBCPP_VERSION
//    std::cout << "Using LLVM libc++ version: " << _LIBCPP_VERSION << std::endl;
//#elif defined(__GLIBCXX__)
//    std::cout << "Using GNU libstdc++" << std::endl;
//#elif defined(_MSVC_STL_VERSION)
//    std::cout << "Using Microsoft STL" << std::endl;
//#else
//    std::cout << "Unknown Standard Library" << std::endl;
//#endif
//}

#include "cth/meta/ranges.hpp"

#include <cth/meta/variadic.hpp>

short test(short) { return {}; }
double test(double) { return {}; }
float test(float) { return {}; }
int test(int) { return {}; }
bool test(bool) { return {}; }

int main() {
    using test_t = std::vector<std::vector<int>>;


    using T = cth::mta::repeat_trait_t<2, std::ranges::range_value_t, test_t>;//cth::mta::md_range_value_t<test_t, 2>;


    std::println("dimensions: {}", cth::mta::dimensions<T>(2));

    std::println("test type: {}", typeid(test_t).name());

    std::println("type: {}", typeid(T).name());
}
