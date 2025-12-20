#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cth::win::io {
/**
 * reads the file without buffering
 * @param path to file
 * @param buffer to fill
 * @attention @ref buffer will be resized, may already have reserved memory
 */
void read_unbuffered(std::string_view path, std::vector<std::byte>& buffer);

inline std::vector<std::byte> read_unbuffered(std::string_view path) {
    std::vector<std::byte> data{};
    read_unbuffered(path, data);
    return data;
}

std::optional<std::string> read_clipboard();
}
