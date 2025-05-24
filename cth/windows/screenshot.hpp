#pragma once
#include "../macro.hpp"
#include "../numeric.hpp"
#include "../io/log.hpp"


#include "win_include.hpp"

#include <glm/glm.hpp>


#include <functional>
#include <span>
#include <type_traits>
#include <vector>

namespace cth::win {

std::vector<uint8_t> screenshot(HWND hwnd, glm::ivec2 size, glm::ivec2 offset);

void screenshot_to(std::span<uint8_t> buffer, HWND hwnd, glm::ivec2 size, glm::ivec2 offset);

}

#include "inl/screenshot.inl"
