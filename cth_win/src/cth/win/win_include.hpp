#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <Psapi.h>

#include "cth/win/win_types.hpp"

namespace cth::win {
using win_file_ptr = std::unique_ptr<std::remove_pointer_t<HANDLE>, handle_deleter>;

using win_global_lock = std::unique_ptr<std::remove_pointer_t<HANDLE>, global_lock_deleter>;



}
