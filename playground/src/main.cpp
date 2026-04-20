#include <iostream>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <vector>
#include <numeric>
#include <print>

#include "cth/meta/concepts.hpp"
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

short test(short) { return {}; }
double test(double) { return {}; }
float test(float) { return {}; }
int test(int) { return {}; }
bool test(bool) { return {}; }



int main() {


    std::vector a{1, 2, 3};

    std::println("{}", a);
}
