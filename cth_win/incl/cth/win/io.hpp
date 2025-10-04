#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cth::win::io {
void read_unbuffered(std::string_view path, std::vector<char>& buffer);

std::optional<std::string> read_clipboard();
}
