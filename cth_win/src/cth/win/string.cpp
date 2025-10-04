#include "cth/win/string.hpp"

#include "cth/io/log.hpp"
#include "cth/win/win_include.hpp"
#include "cth/win/win_types.hpp"

namespace cth::win {

std::wstring to_wstring(std::string_view view) {
    auto const viewSize = static_cast<int>(view.size());
    auto const viewPtr = view.data();

    auto const size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, viewPtr, viewSize, nullptr, 0);
    CTH_STABLE_WIN_THROW(size < 0, "utf16 conversion failed with size [{}]", size) {}

    std::wstring out{};
    out.resize(size);

    auto const result = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        viewPtr,
        viewSize,
        out.data(),
        static_cast<int>(out.size())
    );

    CTH_STABLE_WIN_THROW(result == 0, "utf16 conversion failed") {}
    return out;
}
std::string to_string(std::wstring_view view) {
    auto const viewSize = static_cast<int>(view.size());
    auto const viewPtr = view.data();

    auto const size = WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        viewPtr,
        viewSize,
        nullptr,
        0,
        nullptr,
        nullptr
    );


    CTH_STABLE_WIN_THROW(size < 0, "encoding failed with size [{}]", size) {}


    std::string out{};
    out.resize(size);

    auto const result = WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        viewPtr,
        viewSize,
        out.data(),
        static_cast<int>(out.size()),
        nullptr,
        nullptr
    );

    CTH_STABLE_WIN_THROW(result == 0, "utf8 conversion failed") {}

    return out;
}
}
