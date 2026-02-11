#pragma once
#include "../io/log.hpp"
#include "../macro.hpp"
#include "../numeric.hpp"


#include "win_include.hpp"

#include <glm/glm.hpp>


#include <functional>
#include <span>
#include <type_traits>
#include <vector>

namespace cth::win {
glm::ivec2 extent(HWND hwnd);
glm::ivec2 pos(HWND hwnd);

glm::ivec4 pos_extent(HWND hwnd);

std::vector<uint8_t> screenshot(HWND hwnd, glm::ivec2 size, glm::ivec2 offset);

void screenshot_to(std::span<uint8_t> buffer, HWND hwnd, glm::ivec2 size, glm::ivec2 offset);
}

namespace cth::win {

void reg_global_raw_input(HWND hwnd);

}

#include "inl/window.inl"
