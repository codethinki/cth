// ReSharper disable CppAssignedValueIsNeverUsed
// ReSharper disable CppInitializedValueIsAlwaysRewritten
#pragma once
#include <gtest/gtest.h>

#include "../cth/cth_windows.hpp"


namespace cth::win {

namespace proc {
TEST(func_elevated, main) {
    EXPECT_FALSE(cth::win::proc::elevated());
    int x = win::cmd::hidden("hello");
    x = win::cmd::hidden("hello {}", "asdf");

    x = win::cmd::hidden_dir(std::filesystem::current_path().string(), "hello");
    x = win::cmd::hidden_dir(std::filesystem::current_path().string(), "hello {}", "hello");


}

}

TEST(func_loadTxt, main) {
    try {
        [[maybe_unused]] const auto text = win::file::loadTxt("hello.txt");
        [[maybe_unused]] auto text2 = win::file::loadTxt(L"hello.txt");
    }
    catch(...) {
        std::terminate();
    }
}

inline std::vector<char> loadFileIntoVector(const std::string_view file_path) {
    // Open the file in binary mode at the end of the file
    std::ifstream file(file_path.data(), std::ios::binary | std::ios::ate);
    if(!file) {
        // Handle error or throw an exception
        throw std::runtime_error("Failed to open file");
    }

    // Determine the file size
    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg); // Seek back to the start of the file

    // Reserve space in the vector and read the file
    std::vector<char> buffer(size);
    if(!file.read(buffer.data(), size)) {
        // Handle error or throw an exception
        throw std::runtime_error("Failed to read file: ");
    }

    return buffer;
}

TEST(func_readUnbuffered, main) {


    log::msg("testing io on 1.4mb file");
    constexpr std::string_view smallFilePath = "res/testImage.jpg";


    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();

    std::vector<char> dataUnbuffered{};
    win::file::readUnbuffered(smallFilePath, dataUnbuffered);
    std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();

    log::msg("unbuffered io: {}s", std::chrono::duration<float>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    [[maybe_unused]] std::vector<char> dataBuffered = loadFileIntoVector(smallFilePath);
    end = std::chrono::high_resolution_clock::now();

    log::msg("buffered io: {}s", std::chrono::duration<float>(end - start).count());

    std::vector<char> dataUnbuffered2{};
    std::vector<char> dataBuffered2{};

    log::msg("testing io on 76mb file");
    constexpr std::string_view largeFilePath = "res/Browse.VC.db";


    start = std::chrono::high_resolution_clock::now();

    win::file::readUnbuffered(largeFilePath, dataUnbuffered2);

    end = std::chrono::high_resolution_clock::now();
    log::msg("unbuffered io: {}s", std::chrono::duration<float>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    dataBuffered2 = loadFileIntoVector(largeFilePath);
    end = std::chrono::high_resolution_clock::now();

    log::msg("buffered io: {}s", std::chrono::duration<float>(end - start).count());


}

}
