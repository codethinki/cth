#include "cth/io/log.hpp"

#include "cth/windows.hpp"

#include <fstream>
#include <ios>
#include <vector>
#include <gtest/gtest.h>



namespace cth::win {

std::vector<char> loadFileIntoVector(std::string_view file_path) {
    // Open the io in binary mode at the end of the io
    std::ifstream file(file_path.data(), std::ios::binary | std::ios::ate);
    if(!file) {
        // Handle error or throw an exception
        throw std::runtime_error("Failed to open io");
    }

    // Determine the io size
    std::streamsize const size = file.tellg();
    file.seekg(0, std::ios::beg); // Seek back to the start of the io

    // Reserve space in the vector and read the io
    std::vector<char> buffer(size);
    if(!file.read(buffer.data(), size)) {
        // Handle error or throw an exception
        throw std::runtime_error("Failed to read io: ");
    }

    return buffer;
}


TEST(readUnbuffered, main) {
    log::msg("testing io on 1.4mb io");
    constexpr std::string_view smallFilePath = "res/testImage.jpg";


    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();

    std::vector<char> dataUnbuffered{};
    win::io::readUnbuffered(smallFilePath, dataUnbuffered);
    std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();

    log::msg("unbuffered io: {}s", std::chrono::duration<float>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    [[maybe_unused]] std::vector<char> dataBuffered = loadFileIntoVector(smallFilePath);
    end = std::chrono::high_resolution_clock::now();

    log::msg("buffered io: {}s", std::chrono::duration<float>(end - start).count());

    std::vector<char> dataUnbuffered2{};
    std::vector<char> dataBuffered2{};

    log::msg("testing io on 76mb io");
    constexpr std::string_view largeFilePath = "res/Browse.VC.db";


    start = std::chrono::high_resolution_clock::now();

    win::io::readUnbuffered(largeFilePath, dataUnbuffered2);

    end = std::chrono::high_resolution_clock::now();
    log::msg("unbuffered io: {}s", std::chrono::duration<float>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    dataBuffered2 = loadFileIntoVector(largeFilePath);
    end = std::chrono::high_resolution_clock::now();

    log::msg("buffered io: {}s", std::chrono::duration<float>(end - start).count());


}


} //namespace cth::win
