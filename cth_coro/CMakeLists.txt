glob_cpp(CTH_CORO_SRC "src")
glob_cpp(CTH_CORO_INCL "incl")

add_library(cth_coro STATIC ${CTH_CORO_SRC} ${CTH_CORO_INCL})
add_library(cth::coro ALIAS cth_coro)

package_target_include_directories(
        cth_coro
        PUBLIC incl
)

find_package(Boost REQUIRED COMPONENTS asio)

target_link_libraries(cth_coro PUBLIC Boost::asio cth::cth)
