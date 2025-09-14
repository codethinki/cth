project(cth LANGUAGES CXX)

glob_cpp(CTH_SRC "src")
glob_cpp(CTH_INCL "incl")

add_library(cth STATIC ${CTH_SRC} ${CTH_INCL})
add_library(cth::cth ALIAS cth)

package_target_include_directories(
        cth
        PUBLIC incl
)
