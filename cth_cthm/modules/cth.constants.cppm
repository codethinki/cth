module;
#include "cthm/macro/compilation.hpp"

export module cth.constants;


export namespace cth {
enum class CompilationMode {
    DEBUG,
    RELEASE,
};

constexpr auto COMPILATION_MODE =
#ifdef CTH_DEBUG_MODE
    CompilationMode::DEBUG;
#else
CompilationMode::RELEASE;
#endif

consteval bool debug_mode() {
    return COMPILATION_MODE == CompilationMode::DEBUG;
}

}

