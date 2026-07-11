cth_glob_cpp(CTH_WIN_CAPTURE_SRC "src")
cth_glob_cpp(CTH_WIN_CAPTURE_INCL "incl")

add_library(cth_win_capture STATIC ${CTH_WIN_CAPTURE_SRC} ${CTH_WIN_CAPTURE_INCL})
add_library(cth::win::capture ALIAS cth_win_capture)

cth_target_enable_build_cache(cth_win_capture OPTIONAL)

cth_pkg_target_include_directories(
        cth_win_capture
        PUBLIC incl
        PRIVATE src
)

# exported as cth::win::capture (package component: win_capture)
set_property(TARGET cth_win_capture PROPERTY EXPORT_NAME "win::capture")

target_link_libraries(cth_win_capture PUBLIC cth::win)

# windows sdk libraries, no external packages required
target_link_libraries(cth_win_capture PRIVATE d3d11 dxgi windowsapp)

# tests
if(CTH_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
