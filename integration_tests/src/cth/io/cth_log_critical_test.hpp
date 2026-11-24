#pragma once
#include "cth/test.hpp"

#include "cth/io/log.hpp"

#include <exception>
#include <print>
#include <sstream>

// internal linkage: every test file compiles the checks in its own `CTH_CRITICAL_MODE`
namespace cth::log {
namespace {
    std::ostringstream logs{};
    bool bodyRan = false;

    constexpr int critical(bool violated) {
        CTH_CRITICAL(violated, "critical violated") {
            bodyRan = true;
            details->add("body ran");
        }
        return 1;
    }

    constexpr int asserted(bool holds) {
        CTH_ASSERT(holds, "assert violated") {
            bodyRan = true;
            details->add("body ran");
        }
        return 1;
    }

    [[noreturn]] inline void report() {
        std::print(stderr, "body: {}, log: {}", bodyRan, !logs.view().empty());
        std::abort();
    }

    /**
     * logs into `logs`, the terminate handler prints if the body ran and if the check logged
     */
    inline void report_on_terminate() {
        set_log_stream(logs);
        std::set_terminate(report);
    }
}
}
