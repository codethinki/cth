#include "test.hpp"


#include "cth/windows.hpp"

#include <fstream>
#include <vector>


namespace cth::win::test {


template<class T, class D = float>
struct BenchmarkResult {
    std::chrono::duration<D> duration;
    T result;
};

template<class D = float>
auto bench(auto&& function) {
    auto const start = std::chrono::high_resolution_clock::now();
    auto result = function();
    auto const end = std::chrono::high_resolution_clock::now();

    return BenchmarkResult{
        .duration = std::chrono::duration<D>(end - start),
        .result = result,
    };
}


std::vector<char> loadBuffered(std::string_view file_path) {
    std::ifstream file(file_path.data(), std::ios::binary | std::ios::ate);

    if(!file.is_open())
        std::terminate();

    auto const size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);

    if(!file.read(buffer.data(), size))
        std::terminate();

    return buffer;
}

void bench(std::string_view path) {
    auto const [unbufferedTime, result] = test::bench([path]() {
        std::vector<char> data{};
        win::io::readUnbuffered(path, data);
        return data;
    });

    auto const [bufferedTime, expected] = test::bench([path]() { return test::loadBuffered(path); });

    ASSERT_EQ(result, expected);


    std::println("unbuffered io: {}s", unbufferedTime.count());
    std::println("buffered io: {}s", bufferedTime.count());
}
}


namespace cth::win {

WIN_TEST(readUnbuffered, 1_4_mb) {
    std::println("testing io on 1.4mb io");
    constexpr std::string_view path = "res/testImage.jpg";

    test::bench(path);
}

WIN_TEST(readUnbuffered, 76_1_mb) {
    std::println("testing io on 76mb io");
    constexpr std::string_view path = "res/Browse.VC.db";
    test::bench(path);
}


}
