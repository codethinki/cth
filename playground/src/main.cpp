#include <windows.h>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <vector>
#include <numeric>



int main() {

#ifdef  __cpp_reflection
    std::println("asdf");
#endif


#ifdef _LIBCPP_VERSION
    std::cout << "Using LLVM libc++ version: " << _LIBCPP_VERSION << std::endl;
#elif defined(__GLIBCXX__)
    std::cout << "Using GNU libstdc++" << std::endl;
#elif defined(_MSVC_STL_VERSION)
    std::cout << "Using Microsoft STL" << std::endl;
#else
    std::cout << "Unknown Standard Library" << std::endl;
#endif
}
