#pragma once
#include <boost/asio/error.hpp>

#include "cth/coro/exception.hpp"

namespace bas = boost::asio;

#define BOOST_EC_STABLE_THROW(ec, msg, ...) \
    CTH_STABLE_THROW_T(cth::except::coro_exception, ec.failed(), msg, __VA_ARGS__) {\
        details->add("message: {}", ec.message());\
        details->add("category: {}", ec.category().name());\
        if(ec.has_location()) {\
            auto const& loc = ec.location();\
            details->add("location: {} ({}, {})", loc.file_name(), loc.line(), loc.column());\
        }\
    }
