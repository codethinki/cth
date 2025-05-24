#pragma once
#include "win_include.hpp"

#include <memory>

namespace cth::win {
struct bmp_deleter {
    void operator()(HBITMAP ptr) const { DeleteObject(ptr); }
};
using bmp_ptr = std::unique_ptr<HBITMAP__, bmp_deleter>;
struct hdc_deleter {
    void operator()(HDC ptr) const { DeleteDC(ptr); }
};
using hdc_ptr = std::unique_ptr<HDC__, hdc_deleter>;
}
