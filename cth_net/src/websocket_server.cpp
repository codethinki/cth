#include "cth/net/websocket_server.hpp"

#include "cth/net/exception.hpp"
#include "websocket_connection_state.hpp"

#include <hv/WebSocketServer.h>
#include <hv/hsocket.h>

#include <atomic>
#include <deque>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cth::net {

struct websocket_server::impl {
    hv::WebSocketService service;
    hv::WebSocketServer server{&service};
    std::size_t maxMessageSize;
    std::uint16_t boundPort{};

    mutable std::mutex mutex;
    std::unordered_map<hv::WebSocketChannel*, std::shared_ptr<dev::websocket_connection_state>>
    connections;
    std::deque<std::shared_ptr<dev::websocket_connection_state>> accepted;
    os::fence readyFence;

    std::atomic_bool stopRequested = false;

    impl(std::uint16_t port, std::size_t maxMessageSize);
    ~impl();

    void configure_callbacks();
    void handle_open(WebSocketChannelPtr const& channel);
    void handle_message(WebSocketChannelPtr const& channel, std::string const& data);
    void handle_close(WebSocketChannelPtr const& channel);
    void stop() noexcept;
};

}

namespace cth::net {

websocket_server::impl::impl(std::uint16_t port, std::size_t maxMessageSize) :
    maxMessageSize{maxMessageSize} {
    CTH_CRITICAL(maxMessageSize == 0, "WebSocket maximum message size must be positive") {}

    configure_callbacks();

    auto const listenFd = Listen(port, "127.0.0.1");
    CTH_NET_STABLE_THROW(listenFd < 0, "WebSocket listen failed") {
        auto const error = socket_errno();
        details->add("error: {}", error);
        details->add("message: {}", socket_strerror(error));
    }

    sockaddr_u address{};
    socklen_t addressLength = sizeof(address);
    auto const addressResult = getsockname(listenFd, &address.sa, &addressLength);

    if(addressResult != 0) {
        auto const error = socket_errno();
        closesocket(listenFd);
        CTH_NET_STABLE_THROW(true, "WebSocket local address query failed") {
            details->add("error: {}", error);
            details->add("message: {}", socket_strerror(error));
        }
    }
    boundPort = sockaddr_port(&address);

    server.setHost("127.0.0.1");
    server.setListenFD(listenFd);
    server.setThreadNum(1);

    auto const startResult = server.start();
    CTH_NET_STABLE_THROW(startResult != 0, "WebSocket server start failed") {
        details->add("error: {}", startResult);
    }
}

websocket_server::impl::~impl() { stop(); }

void websocket_server::impl::configure_callbacks() {
    service.onopen = [this](WebSocketChannelPtr const& channel, HttpRequestPtr const&) {
        handle_open(channel);
    };
    service.onmessage = [this](WebSocketChannelPtr const& channel, std::string const& data) {
        handle_message(channel, data);
    };
    service.onclose = [this](WebSocketChannelPtr const& channel) {
        handle_close(channel);
    };
}

void websocket_server::impl::handle_open(WebSocketChannelPtr const& channel) {
    bool reject = false; {
        std::scoped_lock const _{mutex};
        reject = stopRequested.load();
        if(!reject) {
            auto connection = std::make_shared<dev::websocket_connection_state>(
                channel,
                maxMessageSize
            );
            connections.emplace(channel.get(), connection);
            accepted.emplace_back(std::move(connection));
            readyFence.signal();
        }
    }

    if(reject)
        channel->close();
}

void websocket_server::impl::handle_message(
    WebSocketChannelPtr const& channel,
    std::string const& data
) {
    std::shared_ptr<dev::websocket_connection_state> connection; {
        std::scoped_lock const _{mutex};
        auto const found = connections.find(channel.get());
        if(found == connections.end())
            return;
        connection = found->second;
    }
    connection->receive(channel, data);
}

void websocket_server::impl::handle_close(WebSocketChannelPtr const& channel) {
    std::shared_ptr<dev::websocket_connection_state> connection; {
        std::scoped_lock const _{mutex};
        auto const found = connections.find(channel.get());
        if(found == connections.end())
            return;

        connection = std::move(found->second);
        connections.erase(found);
    }
    connection->close(false);
}

void websocket_server::impl::stop() noexcept {
    if(stopRequested.exchange(true))
        return;

    std::vector<std::shared_ptr<dev::websocket_connection_state>> openConnections; {
        std::scoped_lock const _{mutex};
        openConnections.reserve(connections.size());
        for(auto& connection : connections | std::views::values)
            openConnections.emplace_back(std::move(connection));
        connections.clear();
        accepted.clear();
        readyFence.signal();
    }

    for(auto const& connection : openConnections)
        connection->close(true);
    server.stop();
}

}

namespace cth::net {

websocket_server::websocket_server(std::uint16_t port, std::size_t maxMessageSize) :
    _impl{std::make_unique<impl>(port, maxMessageSize)} {}

websocket_server::~websocket_server() = default;

websocket_server::websocket_server(websocket_server&&) noexcept = default;
websocket_server& websocket_server::operator=(websocket_server&&) noexcept = default;

websocket_connection websocket_server::make_connection(
    std::shared_ptr<dev::websocket_connection_state> state
) { return websocket_connection{std::move(state)}; }

std::optional<websocket_connection> websocket_server::accept(std::chrono::milliseconds timeout) {
    CTH_CRITICAL(!_impl, "use after move") {}

    auto const deadline = timeout == os::INFINITE_WAIT
        ? (std::chrono::steady_clock::time_point::max)()
        : std::chrono::steady_clock::now() + timeout;

    while(true) {
        {
            std::scoped_lock const _{_impl->mutex};
            if(!_impl->accepted.empty()) {
                auto connection = make_connection(std::move(_impl->accepted.front()));
                _impl->accepted.pop_front();
                if(_impl->accepted.empty() && !_impl->stopRequested.load())
                    _impl->readyFence.reset();
                return connection;
            }
            if(_impl->stopRequested.load())
                return std::nullopt;
        }

        auto remaining = os::INFINITE_WAIT;
        if(deadline != (std::chrono::steady_clock::time_point::max)()) {
            auto const now = std::chrono::steady_clock::now();
            if(now >= deadline)
                return std::nullopt;
            remaining = std::chrono::ceil<std::chrono::milliseconds>(deadline - now);
        }

        if(_impl->readyFence.wait(remaining) == os::wait_result::TIMEOUT)
            return std::nullopt;
    }
}

void websocket_server::stop() const noexcept {
    if(_impl)
        _impl->stop();
}

std::optional<std::uint16_t> websocket_server::port() const noexcept {
    CTH_CRITICAL(!_impl, "use after move") {}
    return _impl->boundPort;
}

bool websocket_server::stopped() const noexcept {
    CTH_CRITICAL(!_impl, "use after move") {}
    return _impl->stopRequested.load();
}

os::fence const& websocket_server::fence() const {
    CTH_CRITICAL(!_impl, "use after move") {}
    return _impl->readyFence;
}

}
