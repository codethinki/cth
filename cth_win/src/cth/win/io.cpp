#include "cth/win/io.hpp"

#include "cth/macro.hpp"
#include "cth/win/string.hpp"
#include "win_include.hpp"

#include "cth/io/file.hpp"
#include "cth/win/win_types.hpp"


namespace cth::win::io {
void read_unbuffered(std::string_view path, std::vector<std::byte>& buffer) {
    cxpr static size_t PAGE_SIZE = 4096;


    auto const wPath = win::to_wstring(path);

    win_file_ptr const handle{
        CreateFileW(
            wPath.data(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_NO_BUFFERING,
            nullptr
        )
    };

    CTH_WIN_STABLE_THROW(handle.get() == INVALID_HANDLE_VALUE, "failed to create handle for io ({})", path) {}

    auto const fileSize = GetFileSize(handle.get(), nullptr);


    buffer.resize(fileSize + PAGE_SIZE - fileSize % PAGE_SIZE);

    auto const event = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    _OVERLAPPED overlapped{.hEvent = event};

    DWORD bytesRead = 0;
    auto const readFileResult =
        ReadFile(handle.get(), buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, &overlapped);

    CTH_WIN_STABLE_THROW(readFileResult == 0, "failed to read file: {}", path) {}

    auto const queryResult = GetOverlappedResult(handle.get(), &overlapped, &bytesRead, TRUE);


    CTH_WIN_STABLE_THROW(
        queryResult == 0,
        "failed to query overlapped result, file size: {}, buffer size: {}",
        fileSize,
        buffer.size()
    );


    buffer.resize(fileSize);
}

}


namespace cth::win::io {
mapped_file map_file(std::string_view path) {
    auto const wPath = to_wstring(path);
    closing_handle file{
        CreateFileW(
            wPath.data(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        )
    };

    CTH_WIN_STABLE_THROW(file.get() == INVALID_HANDLE_VALUE, "failed to create file handle ({})", path) {}


    auto fileSize = std::filesystem::file_size(wPath);

    closing_handle mapping{CreateFileMappingA(file.get(), nullptr, PAGE_READONLY, 0, 0, nullptr)};
    CTH_WIN_STABLE_THROW(mapping.get() == INVALID_HANDLE_VALUE, "failed to create file mapping ({})", path) {}


    mapping_data_handle dataHandle{
        MapViewOfFile(mapping.get(), FILE_MAP_READ, 0, 0, 0)
    };
    CTH_WIN_STABLE_THROW(dataHandle.get() == nullptr, "failed to map data view of file ({})", path) {}

    auto const* dataPtr = dataHandle.get();

    return {
        std::move(file),
        std::move(mapping),
        std::move(dataHandle),
        {static_cast<std::byte const*>(dataPtr), fileSize}
    };
}


}


namespace cth::win::io {
struct clipbd_access { // NOLINT(cppcoreguidelines-special-member-functions)
    clipbd_access() : _access{static_cast<bool>(OpenClipboard(nullptr))} {}
    ~clipbd_access() {
        if(_access)
            CloseClipboard();
    }
    bool operator!() const noexcept { return !_access; }
    operator bool() const noexcept { return _access; }

private:
    bool _access;
};

std::optional<std::string> read_clipboard() {
    clipbd_access const access{};

    if(!access)
        return std::nullopt;

    auto const data = GetClipboardData(CF_UNICODETEXT);

    if(!data)
        return std::nullopt;

    win_global_lock const textLock{GlobalLock(data)};

    auto const textPtr = static_cast<wchar_t const*>(textLock.get());
    auto const wView = std::wstring_view{textPtr};


    return textPtr != nullptr ? std::optional{to_string(wView)} : std::nullopt;
}
}
