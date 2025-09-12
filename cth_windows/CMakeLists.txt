project(cth_windows LANGUAGES CXX)

glob_cpp(CTH_WINDOWS_SRC "src")
glob_cpp(CTH_WINDOWS_INCL "incl")

add_library(cth_windows STATIC ${CTH_WINDOWS_SRC} ${CTH_WINDOWS_INCL})
add_library(cth_windows::cth_windows ALIAS cth_windows)

target_include_directories(
        cth_windows
        PUBLIC incl
)
