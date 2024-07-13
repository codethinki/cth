#pragma once
#include <codecvt>
#include <fstream>

namespace cth::io::dev {



    template<type::character T>
    std::vector<std::basic_string<T>> loadTxt(const std::basic_string_view<T> path) {
        std::basic_ifstream<T> file(path.data());
        if(std::is_same_v<T, wchar_t>) file.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
        std::basic_string<T> s;
        std::vector<std::basic_string<T>> strings;

        while(std::getline(file, s)) strings.push_back(s);

        return strings;
    }
}
