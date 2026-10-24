#pragma once
#include "cth/coro/utility/fwd.hpp"


namespace cth::co::this_coro {
/**
 * class base for payloads carrying a context
 * @details this coro machinery will auto-inject the context for types inheriting this base
 */
struct context_payload_base {
    constexpr context_payload_base(co::context const& context) : _context{&context} {}

    constexpr co::context const& context() const { return *_context; }

    /**
     * framework use: repoint the payload at the context the coroutine has migrated onto
     * @details keeps every derived field intact, only the context is swapped
     * @param context the coroutine is now running on
     */
    constexpr void rebind_context(co::context const& context) noexcept { _context = &context; }

private:
    co::context const* _context;
};

}
