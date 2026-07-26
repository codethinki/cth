#pragma once
#include "utility/winrt_include.hpp"

#include "cth/win/win_types.hpp"


namespace cth::win::capture::dev {

/**
 * initializes WinRT (MTA) on the calling thread if required
 * @throws cth::except::win_exception on failure
 */
void init_winrt();

/**
 * creates a d3d11 device, falls back to WARP if no hardware device is available
 * @throws cth::except::win_exception on failure
 */
winrt::com_ptr<ID3D11Device> create_d3d_device();

/**
 * wraps a d3d11 device into its winrt projection
 * @throws cth::except::win_exception on failure
 */
wgd3d::IDirect3DDevice to_winrt_device(winrt::com_ptr<ID3D11Device> const& device);

/**
 * creates a capture item for a window
 * @param window to capture
 * @throws cth::except::win_exception on failure
 */
wgc::GraphicsCaptureItem create_item(hwnd_t window);

}
