#include "cth/win/cmd.hpp"

#include "cth/win/string.hpp"
#include "win_include.hpp"

namespace cth::win::cmd {
size_t hidden_dir(std::string_view dir, std::string_view command) {
    PROCESS_INFORMATION pInfo{};
    STARTUPINFOW sInfo{};
    sInfo.cb = sizeof(sInfo);

    auto cmd = std::format("cmd /c \"{}\"", command);
    auto wcmd = win::to_wstring(cmd);
    auto const wdir = win::to_wstring(dir);

    bool const createResult = CreateProcessW(
        nullptr,
        wcmd.data(),
        nullptr,
        nullptr,
        false,
        NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
        nullptr,
        wdir.data(),
        &sInfo,
        &pInfo
    );

    CTH_WIN_WARN(createResult != true, "cmd process creation failed") {
        details->add("cmd: {}", cmd);
        details->add("location: {}", dir);
        details->add("current path: {}", std::filesystem::current_path().string());
        return EXIT_FAILURE;
    }

    closing_handle const hProcess{std::exchange(pInfo.hProcess, nullptr)};
    [[maybe_unused]] closing_handle hThread{std::exchange(pInfo.hThread, nullptr)};


    auto waitResult = WaitForSingleObject(hProcess.get(), INFINITE);

    CTH_WIN_STABLE_THROW(waitResult != 0, "wait for command failed")
    details->add("wait result: {}", waitResult);


    DWORD returnValue = 0;
    GetExitCodeProcess(hProcess.get(), &returnValue);


    return returnValue;
}

}
