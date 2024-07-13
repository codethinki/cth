#pragma once
#include "../cth_concept.hpp"

#include <string>
#include <vector>

namespace cth::io {

    namespace dev {
        template<cth::type::character T>
        std::vector<std::basic_string<T>> loadTxt(std::basic_string_view<T> path);
    }

    inline std::vector<std::string> loadTxt(const std::string_view path) { return dev::loadTxt<char>(path); }
    inline std::vector<std::wstring> loadTxt(const std::wstring_view path) { return dev::loadTxt<wchar_t>(path); }


} //namespace cth::io

#include "inl/cth_read.inl"
