#pragma once

#include "cth/net/exception.hpp"
#include "cth/net/websocket_server.hpp"

#include <cth/coro/awaiters/base/osfence_awaiter_base.hpp>

#include <coroutine>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cth::net::co {

class websocket_connection;



class accept_awaiter : public cth::co::osfence_awaiter_base<> {
public:
    explicit accept_awaiter(net::websocket_server& server) noexcept :
        osfence_awaiter_base{server.fence()},
        _server{server} {}

    [[nodiscard]] std::optional<websocket_connection> await_resume() const;

private:
    net::websocket_server& _server;
};


class message_awaiter_base : public cth::co::osfence_awaiter_base<> {
public:
    explicit message_awaiter_base(net::websocket_connection& connection) noexcept :
        osfence_awaiter_base{connection.fence()},
        _connection{connection} {}

protected:
    net::websocket_connection& _connection;
};

class message_awaiter : public message_awaiter_base {
public:
    using message_awaiter_base::message_awaiter_base;

    [[nodiscard]] bool await_resume() { return _connection.wait(); }
};

class text_awaiter : public message_awaiter_base {
public:
    using message_awaiter_base::message_awaiter_base;

    [[nodiscard]] std::string await_resume() {
        auto const connected = _connection.wait();
        CTH_NET_STABLE_THROW(!connected, "WebSocket closed while awaiting text") {}
        auto const text = _connection.got_text();
        CTH_NET_STABLE_THROW(!text, "WebSocket binary message received as text") {}
        return _connection.read_text();
    }
};

class bytes_awaiter : public message_awaiter_base {
public:
    using message_awaiter_base::message_awaiter_base;

    [[nodiscard]] std::vector<std::byte> await_resume() {
        auto const connected = _connection.wait();
        CTH_NET_STABLE_THROW(!connected, "WebSocket closed while awaiting bytes") {}
        auto const text = _connection.got_text();
        CTH_NET_STABLE_THROW(text, "WebSocket text message received as bytes") {}
        return _connection.read_bytes();
    }
};


class websocket_connection {
public:
    /** waits until a message can be inspected or the connection closes */
    [[nodiscard]] message_awaiter wait() { return message_awaiter{_connection}; }

    /** awaits and consumes the next text message */
    [[nodiscard]] text_awaiter read_text() { return text_awaiter{_connection}; }

    /** awaits and consumes the next binary message */
    [[nodiscard]] bytes_awaiter read_bytes() { return bytes_awaiter{_connection}; }

    /** consumes the text message made ready by @ref wait() */
    [[nodiscard]] std::string take_text() { return _connection.read_text(); }

    /** consumes the binary message made ready by @ref wait() */
    [[nodiscard]] std::vector<std::byte> take_bytes() { return _connection.read_bytes(); }

    void write_text(std::string_view message) const { _connection.write_text(message); }
    void write_bytes(std::span<std::byte const> message) const { _connection.write_bytes(message); }

    void close() const noexcept { _connection.close(); }

    [[nodiscard]] bool got_text() const { return _connection.got_text(); }
    [[nodiscard]] bool connected() const noexcept { return _connection.connected(); }
    [[nodiscard]] os::fence const& fence() const { return _connection.fence(); }

private:
    friend class accept_awaiter;

    explicit websocket_connection(net::websocket_connection connection) :
        _connection{std::move(connection)} {}

    net::websocket_connection _connection;

public:
    websocket_connection(websocket_connection const&) = delete;
    websocket_connection& operator=(websocket_connection const&) = delete;
    websocket_connection(websocket_connection&&) noexcept = default;
    websocket_connection& operator=(websocket_connection&&) noexcept = default;
};

inline std::optional<websocket_connection> accept_awaiter::await_resume() const {
    auto connection = _server.accept();
    if(!connection)
        return std::nullopt;
    websocket_connection result{std::move(*connection)};
    return {std::move(result)};
}


class websocket_server {
public:
    explicit websocket_server(
        std::uint16_t port = 0,
        std::size_t maxMessageSize = net::websocket_server::DEFAULT_MAX_MESSAGE_SIZE
    ) : _server{port, maxMessageSize} {}

    /** awaits and consumes the next accepted connection */
    [[nodiscard]] accept_awaiter accept() { return accept_awaiter{_server}; }

    void stop() const noexcept { _server.stop(); }

    [[nodiscard]] std::optional<std::uint16_t> port() const noexcept { return _server.port(); }
    [[nodiscard]] bool stopped() const noexcept { return _server.stopped(); }
    [[nodiscard]] os::fence const& fence() const { return _server.fence(); }

private:
    net::websocket_server _server;

public:
    websocket_server(websocket_server const&) = delete;
    websocket_server& operator=(websocket_server const&) = delete;
    websocket_server(websocket_server&&) noexcept = default;
    websocket_server& operator=(websocket_server&&) noexcept = default;
};

}
