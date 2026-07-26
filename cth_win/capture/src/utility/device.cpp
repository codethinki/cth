#include "utility/device.hpp"


namespace cth::win::capture::dev {

void init_winrt() {
    auto const result = RoInitialize(RO_INIT_MULTITHREADED);
    CTH_WIN_STABLE_THROW(FAILED(result) && result != RPC_E_CHANGED_MODE, "failed to initialize winrt") {}
}

winrt::com_ptr<ID3D11Device> create_d3d_device() {
    winrt::com_ptr<ID3D11Device> device{};

    auto const create = [&device](D3D_DRIVER_TYPE driver) {
        return D3D11CreateDevice(
            nullptr,
            driver,
            nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            device.put(),
            nullptr,
            nullptr
        );
    };

    auto result = create(D3D_DRIVER_TYPE_HARDWARE);
    if(result == static_cast<HRESULT>(DXGI_ERROR_UNSUPPORTED))
        result = create(D3D_DRIVER_TYPE_WARP);

    CTH_WIN_STABLE_THROW(FAILED(result), "failed to create d3d11 device") {}
    return device;
}

wgd3d::IDirect3DDevice to_winrt_device(winrt::com_ptr<ID3D11Device> const& device) {
    winrt::com_ptr<IInspectable> inspectable{};

    auto const result = CreateDirect3D11DeviceFromDXGIDevice(
        device.as<IDXGIDevice>().get(),
        inspectable.put()
    );
    CTH_WIN_STABLE_THROW(FAILED(result), "failed to create winrt d3d11 device") {}

    return inspectable.as<wgd3d::IDirect3DDevice>();
}

wgc::GraphicsCaptureItem create_item(hwnd_t window) {
    auto const interop = winrt::get_activation_factory<wgc::GraphicsCaptureItem>()
        .as<IGraphicsCaptureItemInterop>();

    wgc::GraphicsCaptureItem item{nullptr};
    auto const result = interop->CreateForWindow(
        static_cast<HWND>(window),
        winrt::guid_of<wgc::GraphicsCaptureItem>(),
        winrt::put_abi(item)
    );
    CTH_WIN_STABLE_THROW(FAILED(result), "failed to create capture item for [{}]", window) {}

    return item;
}

}
