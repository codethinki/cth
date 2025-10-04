#include "cth/win/cmd.hpp"

#include "cth/win/string.hpp"
#include "cth/win/win_include.hpp"

namespace cth::win::cmd {
int hidden_dir(std::string_view dir, std::string_view command) {
    PROCESS_INFORMATION pInfo{};
    STARTUPINFOW sInfo{};
    sInfo.cb = sizeof(sInfo);

    auto cmd = std::format("cmd /c \"{}\"", command);
    auto wcmd = win::to_wstring(cmd);
    auto const wdir = win::to_wstring(dir);

    bool const res = CreateProcessW(
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


    CTH_WARN(res != 1, "cmd process creation failed") {
        details->add("cmd: {}", cmd);
        details->add("location: {}", dir);
        return EXIT_FAILURE;
    }


    WaitForSingleObject(pInfo.hProcess, INFINITE);

    DWORD returnValue = 0;
    GetExitCodeProcess(pInfo.hProcess, &returnValue);

    CloseHandle(pInfo.hProcess);
    CloseHandle(pInfo.hThread);

    return static_cast<int>(returnValue);
}

}
