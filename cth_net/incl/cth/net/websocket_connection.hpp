#pragma once

#include <cth/os/fence.hpp>

#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cth::net::dev {
struct websocket_connection_state;
}

namespace cth::net {

class websocket_connection {
public:
    websocket_connection() = default;
    ~websocket_connection() = default;

    /** blocks until a message is ready or the connection closes */
    [[nodiscard]] bool wait();

    /** consumes the message made available by wait() */
    [[nodiscard]] std::string read_text();
    [[nodiscard]] std::vector<std::byte> read_bytes();

    /** queues a message for transmission */
    void write_text(std::string_view message) const;
    void write_bytes(std::span<std::byte const> message) const;

    void close() const noexcept;

    [[nodiscard]] bool got_text() const;
    [[nodiscard]] bool connected() const noexcept;

    /**
     * message readiness fence
     * @return a manual-reset fence signaled while a message is queued or the connection is closed
     */
    [[nodiscard]] os::fence const& fence() const;

private:
    friend class websocket_server;

    explicit websocket_connection(std::shared_ptr<dev::websocket_connection_state> state) :
        _state{std::move(state)} {}

    std::shared_ptr<dev::websocket_connection_state> _state;

public:
    websocket_connection(websocket_connection const&) = delete;
    websocket_connection& operator=(websocket_connection const&) = delete;
    websocket_connection(websocket_connection&&) noexcept = default;
    websocket_connection& operator=(websocket_connection&&) noexcept = default;
};

}
