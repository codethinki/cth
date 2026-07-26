#include "cth/win/capture.hpp"

#include "utility/device.hpp"
#include "utility/winrt_include.hpp"

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <utility>
#include <vector>


namespace cth::win::capture {
namespace {
    [[noreturn]] void rethrow_winrt(winrt::hresult_error const& error, std::string_view what) {
        CTH_WIN_STABLE_THROW(true, "{}: {}", what, winrt::to_string(error.message())) {}
        std::unreachable();
    }
}


struct window_capture::impl {
    explicit impl(hwnd_t window) {
        dev::init_winrt();

        CTH_WIN_STABLE_THROW(
            !wgc::GraphicsCaptureSession::IsSupported(),
            "window capture not supported (requires >= WIN10 1903)"
        ) {}

        d3DDevice = dev::create_d3d_device();
        d3DDevice->GetImmediateContext(d3DContext.put());
        device = dev::to_winrt_device(d3DDevice);

        item = dev::create_item(window);
        poolSize = item.Size();

        framePool = wgc::Direct3D11CaptureFramePool::CreateFreeThreaded(
            device,
            wgd::DirectXPixelFormat::B8G8R8A8UIntNormalized,
            2,
            poolSize
        );
        session = framePool.CreateCaptureSession(item);

        frameRevoker = framePool.FrameArrived(
            winrt::auto_revoke,
            [this](auto&&, auto&&) {
                {
                    std::scoped_lock const lock{mtx};
                    frameArrived = true;
                }
                cv.notify_all();
            }
        );
        closedRevoker = item.Closed(
            winrt::auto_revoke,
            [this](auto&&, auto&&) {
                closed = true;
                cv.notify_all();
            }
        );

        remove_border();

        session.StartCapture();
    }
    ~impl() {
        frameRevoker.revoke();
        closedRevoker.revoke();
        if(session)
            session.Close();
        if(framePool)
            framePool.Close();
    }

    /**
     * best-effort: drops the capture border
     * @details requires IGraphicsCaptureSession3 (>= WIN10 20348) and user consent,
     * silently keeps the border otherwise
     */
    void remove_border() {
        namespace meta = winrt::Windows::Foundation::Metadata;
        namespace cap = winrt::Windows::Security::Authorization::AppCapabilityAccess;
        try {
            if(!meta::ApiInformation::IsPropertyPresent(
                winrt::name_of<wgc::GraphicsCaptureSession>(),
                L"IsBorderRequired"
            ))
                return;

            auto const status = wgc::GraphicsCaptureAccess::RequestAccessAsync(
                wgc::GraphicsCaptureAccessKind::Borderless
            ).get();
            if(status != cap::AppCapabilityAccessStatus::Allowed)
                return;

            session.IsBorderRequired(false);
        } catch(winrt::hresult_error const&) {} // border stays, capture still works
    }

    /**
     * drains the frame pool
     * @return newest frame or nullptr
     */
    wgc::Direct3D11CaptureFrame latest_frame() {
        wgc::Direct3D11CaptureFrame latest{nullptr};

        while(true) {
            auto frame = framePool.TryGetNextFrame();
            if(!frame)
                break;
            if(latest)
                latest.Close();
            latest = std::move(frame);
        }

        return latest;
    }

    /**
     * copies the frame into @ref buffer via a staging texture
     */
    void store(wgc::Direct3D11CaptureFrame const& frame) {
        winrt::com_ptr<ID3D11Texture2D> texture{};
        auto const access = frame.Surface()
                                 .as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();

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
            auto const stagingRes = d3DDevice->CreateTexture2D(&stagingDesc, nullptr, staging.put());
            CTH_WIN_STABLE_THROW(FAILED(stagingRes), "failed to create staging texture") {}

            width = desc.Width;
            height = desc.Height;
        }

        d3DContext->CopyResource(staging.get(), texture.get());

        D3D11_MAPPED_SUBRESOURCE mapped{};
        auto const mapRes = d3DContext->Map(staging.get(), 0, D3D11_MAP_READ, 0, &mapped);
        CTH_WIN_STABLE_THROW(FAILED(mapRes), "failed to map staging texture") {}

        auto const rowBytes = width * PIXEL_SIZE;
        buffer.resize(height * rowBytes);

        auto const* src = static_cast<std::byte const*>(mapped.pData);
        for(size_t y = 0; y < height; ++y)
            std::memcpy(buffer.data() + y * rowBytes, src + y * mapped.RowPitch, rowBytes);

        d3DContext->Unmap(staging.get(), 0);

        // window resized => resize the pool, applies to future frames
        auto const contentSize = frame.ContentSize();
        if(contentSize.Width != poolSize.Width || contentSize.Height != poolSize.Height) {
            poolSize = contentSize;
            framePool.Recreate(device, wgd::DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, poolSize);
        }
    }

    std::span<std::byte> try_frame() {
        auto const frame = latest_frame();
        if(!frame)
            return {};

        store(frame);
        frame.Close();

        return {buffer.data(), buffer.size()};
    }

    std::span<std::byte> next_frame(std::chrono::milliseconds timeout) {
        std::unique_lock lock{mtx};
        auto const pred = [this] { return frameArrived || closed.load(); };

        if(timeout.count() == 0)
            cv.wait(lock, pred);
        else if(!cv.wait_for(lock, timeout, pred))
            return {};

        frameArrived = false;
        lock.unlock();

        return try_frame();
    }


    winrt::com_ptr<ID3D11Device> d3DDevice{};
    winrt::com_ptr<ID3D11DeviceContext> d3DContext{};
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
    dev::init_winrt();

    try { return wgc::GraphicsCaptureSession::IsSupported(); } catch(winrt::hresult_error const&) { return false; }
}


window_capture::window_capture(hwnd_t window, config_t config) {
    try { _impl = std::make_unique<impl>(window); } catch(winrt::hresult_error const& e) {
        rethrow_winrt(e, "failed to start window capture");
    }

    capture_cursor(config.captureCursor);
}
window_capture::~window_capture() = default;


std::span<std::byte> window_capture::next_frame(std::chrono::milliseconds timeout) {
    try { return _impl->next_frame(timeout); } catch(winrt::hresult_error const& e) {
        rethrow_winrt(e, "failed to capture frame");
    }
}
std::span<std::byte> window_capture::try_frame() {
    try { return _impl->try_frame(); } catch(winrt::hresult_error const& e) {
        rethrow_winrt(e, "failed to capture frame");
    }
}
void window_capture::capture_cursor(bool enable) {
    try { _impl->session.IsCursorCaptureEnabled(enable); } catch(winrt::hresult_error const& e) {
        rethrow_winrt(e, "failed to set cursor capture");
    }
}

bool window_capture::closed() const { return _impl->closed; }

size_t window_capture::width() const { return _impl->width; }
size_t window_capture::height() const { return _impl->height; }

std::span<std::byte> window_capture::view() { return {_impl->buffer.data(), _impl->buffer.size()}; }
std::span<std::byte const> window_capture::view() const { return {_impl->buffer.data(), _impl->buffer.size()}; }

window_capture::window_capture(window_capture&&) noexcept = default;
window_capture& window_capture::operator=(window_capture&&) noexcept = default;
}
