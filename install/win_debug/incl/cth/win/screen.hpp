#pragma once

#include "win_types.hpp"

#include "cth/io/log.hpp"



namespace cth::win::screen {

/**
 * creates a windows window
 * @param name of window (in utf8), must not be empty
 * @param rect pos + size
 * @param class_name (in utf8) if empty => name is used, class doesn't exist => created
 * @throws cth::except::win_exception on window class registration failure
 * @throws cth::except::win_exception on window creation failure
 */
window_t create_window(std::string_view name, rect_t rect, bool visible = true, std::string_view class_name = {});


/**
 * Enumerates all monitors
 */
std::vector<monitor_t> enum_monitors();

/** 
 * Gets the desktop rect, origin top left
 */
rect_t desktop_rect();
} // namespace cth::win
