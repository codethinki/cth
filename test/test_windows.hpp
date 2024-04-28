#pragma once
#include <gtest/gtest.h>

#include "../cth/cth_windows.hpp"


namespace cth {


TEST(headerWindows, funcIsElevatedProc) {
    EXPECT_FALSE(cth::win::proc::elevated());

    auto x = win::cmd::hidden("asdfasdf");
    x = win::cmd::hidden("asdfaasdf {}", "asdf");

    x = win::cmd::hidden_dir(std::filesystem::current_path().string(), "asdfasdf");
    x = win::cmd::hidden_dir(std::filesystem::current_path().string(), "asdfasdf {}", "hello");
}
TEST(headerWindows, funcLoadTxt) {
    try {
        auto text = win::file::loadTxt("hello.txt");
        if(!text.empty())cout << text[0];
        auto wtext = win::file::loadTxt(L"hello.txt");
    }
    catch(...) {}
}

std::vector<char> loadFileIntoVector(const string_view filePath) {
    // Open the file in binary mode at the end of the file
    std::ifstream file(filePath.data(), std::ios::binary | std::ios::ate);
    if(!file) {
        // Handle error or throw an exception
        throw std::runtime_error("Failed to open file");
    }

    // Determine the file size
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg); // Seek back to the start of the file

    // Reserve space in the vector and read the file
    std::vector<char> buffer(size);
    if(!file.read(buffer.data(), size)) {
        // Handle error or throw an exception
        throw std::runtime_error("Failed to read file: ");
    }

    return buffer;
}

TEST(headerWindows, readUnbuffered) {

    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;


    log::msg("testing io on 1.4mb file");
    constexpr string_view smallFilePath = "res/testImage.jpg";
    

    vector<char> dataUnbuffered{};
    vector<char> dataBuffered{};

    start = std::chrono::high_resolution_clock::now();

    win::file::readUnbuffered(smallFilePath, dataUnbuffered);
    end = std::chrono::high_resolution_clock::now();

    log::msg("unbuffered io: {}s", std::chrono::duration<float>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    dataBuffered = loadFileIntoVector(smallFilePath);
    end = std::chrono::high_resolution_clock::now();

    log::msg("buffered io: {}s", std::chrono::duration<float>(end - start).count());

    vector<char> dataUnbuffered2{};
    vector<char> dataBuffered2{};

    log::msg("testing io on 76mb file");
    constexpr string_view largeFilePath = "res/Browse.VC.db";


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
