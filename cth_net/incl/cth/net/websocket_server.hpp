#pragma once

#include "cth/net/websocket_connection.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

namespace cth::net {

/** WebSocket server bound to the IPv4 loopback interface. */
class websocket_server {
public:
    static constexpr std::size_t DEFAULT_MAX_MESSAGE_SIZE = 1024 * 1024;

    explicit websocket_server(
        std::uint16_t port = 0,
        std::size_t maxMessageSize = DEFAULT_MAX_MESSAGE_SIZE
    );
    ~websocket_server();

    /**
     * blocks until a connection is accepted, the server stops, or the timeout elapses
     * @return the connection, or @ref std::nullopt if stopped or timed out
     */
    [[nodiscard]] std::optional<websocket_connection> accept(
        std::chrono::milliseconds timeout = os::INFINITE_WAIT
    );

    /** stops accepting and closes all connections */
    void stop() const noexcept;

    [[nodiscard]] std::optional<std::uint16_t> port() const noexcept;
    [[nodiscard]] bool stopped() const noexcept;

    /**
     * connection readiness fence
     * @return a manual-reset fence signaled while a connection is queued or the server is stopped
     */
    [[nodiscard]] os::fence const& fence() const;

private:
    struct impl;
    std::unique_ptr<impl> _impl;

    [[nodiscard]] static websocket_connection make_connection(
        std::shared_ptr<dev::websocket_connection_state> state
    );

public:
    websocket_server(websocket_server const&) = delete;
    websocket_server& operator=(websocket_server const&) = delete;
    websocket_server(websocket_server&&) noexcept;
    websocket_server& operator=(websocket_server&&) noexcept;
};

}
