#include "cth/win/capture.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

// required before winrt headers for classic COM interop
#include <unknwn.h>

#include <d3d11.h>
#include <roapi.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>

#include <Windows.Graphics.Capture.Interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <utility>
#include <vector>


namespace cth::win::capture {
namespace {
    namespace wgc = winrt::Windows::Graphics::Capture;
    namespace wgd = winrt::Windows::Graphics::DirectX;
    namespace wgd3d = winrt::Windows::Graphics::DirectX::Direct3D11;


    void init_winrt() {
        auto const result = RoInitialize(RO_INIT_MULTITHREADED);
        CTH_WIN_STABLE_THROW(FAILED(result) && result != RPC_E_CHANGED_MODE, "failed to initialize winrt") {}
    }

    [[noreturn]] void rethrow_winrt(winrt::hresult_error const& error, std::string_view what) {
        CTH_WIN_STABLE_THROW(true, "{}: {}", what, winrt::to_string(error.message())) {}
        std::unreachable();
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

        auto const result =
            CreateDirect3D11DeviceFromDXGIDevice(device.as<IDXGIDevice>().get(), inspectable.put());
        CTH_WIN_STABLE_THROW(FAILED(result), "failed to create winrt d3d11 device") {}

        return inspectable.as<wgd3d::IDirect3DDevice>();
    }

    wgc::GraphicsCaptureItem create_item(hwnd_t window) {
        auto const interop =
            winrt::get_activation_factory<wgc::GraphicsCaptureItem>().as<IGraphicsCaptureItemInterop>();

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


struct window_capture::impl {
    explicit impl(hwnd_t window) {
        init_winrt();

        CTH_WIN_STABLE_THROW(
            !wgc::GraphicsCaptureSession::IsSupported(),
            "window capture not supported (requires >= WIN10 1903)"
        ) {}

        d3dDevice = create_d3d_device();
        d3dDevice->GetImmediateContext(d3dContext.put());
        device = to_winrt_device(d3dDevice);

        item = create_item(window);
        poolSize = item.Size();

        framePool = wgc::Direct3D11CaptureFramePool::CreateFreeThreaded(
            device,
            wgd::DirectXPixelFormat::B8G8R8A8UIntNormalized,
            2,
            poolSize
        );
        session = framePool.CreateCaptureSession(item);

        frameRevoker = framePool.FrameArrived(winrt::auto_revoke, [this](auto&&, auto&&) {
            {
                std::scoped_lock const lock{mtx};
                frameArrived = true;
            }
            cv.notify_all();
        });
        closedRevoker = item.Closed(winrt::auto_revoke, [this](auto&&, auto&&) {
            closed = true;
            cv.notify_all();
        });

        session.StartCapture();
    }
    ~impl() {
        frameRevoker.revoke();
        closedRevoker.revoke();
        if(session) session.Close();
        if(framePool) framePool.Close();
    }

    /**
     * drains the frame pool
     * @return newest frame or nullptr
     */
    wgc::Direct3D11CaptureFrame latest_frame() {
        wgc::Direct3D11CaptureFrame latest{nullptr};

        while(true) {
            auto frame = framePool.TryGetNextFrame();
            if(!frame) break;
            if(latest) latest.Close();
            latest = std::move(frame);
        }

        return latest;
    }

    /**
     * copies the frame into @ref buffer via a staging texture
     */
    void store(wgc::Direct3D11CaptureFrame const& frame) {
        winrt::com_ptr<ID3D11Texture2D> texture{};
        auto const access = frame.Surface().as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();

        auto const textureRes = access->GetInterface(winrt::guid_of<ID3D11Texture2D>(), texture.put_void());
        CTH_WIN_STABLE_THROW(FAILED(textureRes), "failed to get frame texture") {}

        D3D11_TEXTURE2D_DESC desc{};
        texture->GetDesc(&desc);

        if(!staging || desc.Width != width || desc.Height != height) {
            auto stagingDesc = desc;
            stagingDesc.Usage = D3D11_USAGE_STAGING;
            stagingDesc.BindFlags = 0;
            stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            stagingDesc.MiscFlags = 0;

            staging = nullptr;
            auto const stagingRes = d3dDevice->CreateTexture2D(&stagingDesc, nullptr, staging.put());
            CTH_WIN_STABLE_THROW(FAILED(stagingRes), "failed to create staging texture") {}

            width = desc.Width;
            height = desc.Height;
        }

        d3dContext->CopyResource(staging.get(), texture.get());

        D3D11_MAPPED_SUBRESOURCE mapped{};
        auto const mapRes = d3dContext->Map(staging.get(), 0, D3D11_MAP_READ, 0, &mapped);
        CTH_WIN_STABLE_THROW(FAILED(mapRes), "failed to map staging texture") {}

        auto const rowBytes = width * PIXEL_SIZE;
        buffer.resize(height * rowBytes);

        auto const* src = static_cast<std::byte const*>(mapped.pData);
        for(size_t y = 0; y < height; ++y)
            std::memcpy(buffer.data() + y * rowBytes, src + y * mapped.RowPitch, rowBytes);

        d3dContext->Unmap(staging.get(), 0);

        // window resized => resize the pool, applies to future frames
        auto const contentSize = frame.ContentSize();
        if(contentSize.Width != poolSize.Width || contentSize.Height != poolSize.Height) {
            poolSize = contentSize;
            framePool.Recreate(device, wgd::DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, poolSize);
        }
    }

    std::span<std::byte> try_frame() {
        auto const frame = latest_frame();
        if(!frame) return {};

        store(frame);
        frame.Close();

        return {buffer.data(), buffer.size()};
    }

    std::span<std::byte> next_frame(std::chrono::milliseconds timeout) {
        std::unique_lock lock{mtx};
        auto const pred = [this] { return frameArrived || closed.load(); };

        if(timeout.count() == 0) cv.wait(lock, pred);
        else if(!cv.wait_for(lock, timeout, pred)) return {};

        frameArrived = false;
        lock.unlock();

        return try_frame();
    }


    winrt::com_ptr<ID3D11Device> d3dDevice{};
    winrt::com_ptr<ID3D11DeviceContext> d3dContext{};
    wgd3d::IDirect3DDevice device{nullptr};

    wgc::GraphicsCaptureItem item{nullptr};
    wgc::Direct3D11CaptureFramePool framePool{nullptr};
    wgc::GraphicsCaptureSession session{nullptr};

    wgc::Direct3D11CaptureFramePool::FrameArrived_revoker frameRevoker{};
    wgc::GraphicsCaptureItem::Closed_revoker closedRevoker{};

    winrt::com_ptr<ID3D11Texture2D> staging{};
    winrt::Windows::Graphics::SizeInt32 poolSize{};

    std::vector<std::byte> buffer{};
    size_t width = 0;
    size_t height = 0;

    std::mutex mtx{};
    std::condition_variable cv{};
    bool frameArrived = false;
    std::atomic_bool closed = false;
};


bool supported() {
    init_winrt();

    try {
        return wgc::GraphicsCaptureSession::IsSupported();
    } catch(winrt::hresult_error const&) { return false; }
}


window_capture::window_capture(hwnd_t window) {
    try {
        _impl = std::make_unique<impl>(window);
    } catch(winrt::hresult_error const& e) { rethrow_winrt(e, "failed to start window capture"); }
}
window_capture::~window_capture() = default;
window_capture::window_capture(window_capture&&) noexcept = default;
window_capture& window_capture::operator=(window_capture&&) noexcept = default;

std::span<std::byte> window_capture::next_frame(std::chrono::milliseconds timeout) {
    try {
        return _impl->next_frame(timeout);
    } catch(winrt::hresult_error const& e) { rethrow_winrt(e, "failed to capture frame"); }
}
std::span<std::byte> window_capture::try_frame() {
    try {
        return _impl->try_frame();
    } catch(winrt::hresult_error const& e) { rethrow_winrt(e, "failed to capture frame"); }
}
void window_capture::capture_cursor(bool enable) {
    try {
        _impl->session.IsCursorCaptureEnabled(enable);
    } catch(winrt::hresult_error const& e) { rethrow_winrt(e, "failed to set cursor capture"); }
}

bool window_capture::closed() const { return _impl->closed; }

size_t window_capture::width() const { return _impl->width; }
size_t window_capture::height() const { return _impl->height; }

std::span<std::byte> window_capture::view() { return {_impl->buffer.data(), _impl->buffer.size()}; }
std::span<std::byte const> window_capture::view() const {
    return {_impl->buffer.data(), _impl->buffer.size()};
}

}
