cth_glob_cpp(PLAYGROUND_SRC "src")

add_executable(playground ${PLAYGROUND_SRC})

target_include_directories(
        playground
        PRIVATE src
)
find_package(Boost REQUIRED COMPONENTS asio)

target_link_libraries(playground PRIVATE cth::cth Boost::asio)
