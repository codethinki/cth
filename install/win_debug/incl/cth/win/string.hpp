#pragma once
#include <string>


namespace cth::win {
/**
 * converts an utf-8 view to an utf-16 string
 * @param view must be valid utf-8, may be null terminated
 */
std::wstring to_wstring(std::string_view view);
/**
 * converts a utf-8 view to an utf-16 string
 * @param view must be valid utf-16, may be null terminated
 */
std::string to_string(std::wstring_view view);

}
