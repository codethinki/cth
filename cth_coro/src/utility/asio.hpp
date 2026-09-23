#pragma once
#include <asio/error_code.hpp>

#include "cth/coro/exception.hpp"

namespace bas = asio;

#define ASIO_EC_STABLE_THROW(ec, msg, ...)                                         \
    CTH_STABLE_THROW_T(cth::except::coro_exception, ec, msg, __VA_ARGS__) {        \
        details->add("message: {}", ec.message());                                \
        details->add("category: {}", ec.category().name());                       \
    }
