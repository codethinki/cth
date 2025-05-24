#pragma once
#include "log.hpp"

#include <filesystem>
#include <fstream>
#include <ios>


namespace cth::io::file {
cxpr uintmax_t BYTE = 1;
cxpr uintmax_t KB = BYTE * 1024;
cxpr uintmax_t MB = KB * 1024;
cxpr uintmax_t GB = MB * 1024;


template<uintmax_t Divisor = KB>
uintmax_t size(std::filesystem::path const& path) {
    if(!std::filesystem::exists(path)) return -1;

    return file_size(path) / Divisor;
}

template<class D, size_t Buffer = 0xffff>
std::vector<std::string> chop(std::filesystem::path const& path, D delimiter) {
    CTH_STABLE_ERR(!std::filesystem::exists(path), "file doesn't exist [{}]", path.string()) {
        throw details->exception();
    }

    std::vector<std::string> result{};
    std::ifstream file{path};


    std::array<char, Buffer> buffer;
    file.rdbuf()->pubsetbuf(buffer.data(), buffer.size());

    std::string line;
    while(std::getline(file, line, delimiter))
        result.push_back(line);

    file.close();


    if(!file.eof() && !line.empty()) result.push_back(line);


    return result;
}

template<size_t Buffer = 0xffff>
std::vector<std::string> chop(std::filesystem::path const& path, char delimiter = '\n') {
    return chop<char, Buffer>(path, delimiter);
}




}
