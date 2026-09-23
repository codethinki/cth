#include "cth/net/websocket_connection.hpp"

#include "cth/net/exception.hpp"
#include "websocket_connection_state.hpp"

#include <hv/WebSocketChannel.h>

#include <limits>
#include <utility>

namespace cth::net::dev {

websocket_connection_state::websocket_connection_state(
    WebSocketChannelPtr channel,
    std::size_t maxMessageSize
) : channel{std::move(channel)}, maxMessageSize{maxMessageSize} {}

void websocket_connection_state::receive(
    WebSocketChannelPtr const& source,
    std::string const& data
) {
    bool oversized = false;
    {
        std::scoped_lock const _{mutex};
        if(closed || source != channel)
            return;

        oversized = data.size() > maxMessageSize;
        if(oversized) {
            closed = true;
            channel.reset();
        } else {
            messages.emplace_back(websocket_message{
                .bytes = std::vector<std::byte>{
                    reinterpret_cast<std::byte const*>(data.data()),
                    reinterpret_cast<std::byte const*>(data.data() + data.size())
                },
                .text = source->opcode == WS_OPCODE_TEXT
            });
        }
        readyFence.signal();
    }

    if(!oversized)
        return;

    CTH_NET_WARN(true, "WebSocket message exceeds the configured maximum size") {
        details->add("size: {}", data.size());
        details->add("maximum: {}", maxMessageSize);
    }
    source->close();
}

void websocket_connection_state::close(bool closeChannel) noexcept {
    WebSocketChannelPtr openChannel; {
        std::scoped_lock const _{mutex};
        if(closed)
            return;

        closed = true;
        openChannel = std::move(channel);
        readyFence.signal();
    }

    if(closeChannel && openChannel)
        openChannel->close();
}

}

namespace cth::net {

bool websocket_connection::wait() {
    CTH_CRITICAL(!_state, "use after move") {}

    while(true) {
        {
            std::scoped_lock const _{_state->mutex};
            if(!_state->messages.empty())
                return true;
            if(_state->closed)
                return false;
        }
        _state->readyFence.wait();
    }
}

std::vector<std::byte> websocket_connection::read_bytes() {
    CTH_CRITICAL(!_state, "use after move") {}

    std::scoped_lock const _{_state->mutex};
    CTH_CRITICAL(_state->messages.empty(), "no WebSocket message ready") {}

    auto result = std::move(_state->messages.front().bytes);
    _state->messages.pop_front();
    if(_state->messages.empty() && !_state->closed)
        _state->readyFence.reset();
    return result;
}

std::string websocket_connection::read_text() {
    auto message = read_bytes();
    if(message.empty())
        return {};
    return {reinterpret_cast<char const*>(message.data()), message.size()};
}

void websocket_connection::write_text(std::string_view message) const {
    CTH_CRITICAL(!_state, "use after move") {}
    CTH_CRITICAL(
        message.size() > (std::numeric_limits<int>::max)(),
        "WebSocket message is too large"
    ) {}

    WebSocketChannelPtr channel; {
        std::scoped_lock const _{_state->mutex};
        channel = _state->channel;
    }
    CTH_CRITICAL(!channel, "WebSocket connection is closed") {}

    auto const result = channel->send(message.data(), static_cast<int>(message.size()), WS_OPCODE_TEXT);
    CTH_NET_STABLE_THROW(result < 0, "WebSocket text write failed") { details->add("error: {}", result); }
}

void websocket_connection::write_bytes(std::span<std::byte const> message) const {
    CTH_CRITICAL(!_state, "use after move") {}
    CTH_CRITICAL(
        message.size() > (std::numeric_limits<int>::max)(),
        "WebSocket message is too large"
    ) {}

    WebSocketChannelPtr channel; {
        std::scoped_lock const _{_state->mutex};
        channel = _state->channel;
    }
    CTH_CRITICAL(!channel, "WebSocket connection is closed") {}

    auto const result = channel->send(
        reinterpret_cast<char const*>(message.data()),
        static_cast<int>(message.size()),
        WS_OPCODE_BINARY
    );
    CTH_NET_STABLE_THROW(result < 0, "WebSocket binary write failed") { details->add("error: {}", result); }
}

void websocket_connection::close() const noexcept {
    if(_state)
        _state->close(true);
}

bool websocket_connection::got_text() const {
    CTH_CRITICAL(!_state, "use after move") {}

    std::scoped_lock const _{_state->mutex};
    CTH_CRITICAL(_state->messages.empty(), "no WebSocket message ready") {}
    return _state->messages.front().text;
}

bool websocket_connection::connected() const noexcept {
    CTH_CRITICAL(!_state, "use after move") {}

    std::scoped_lock const _{_state->mutex};
    return !_state->closed;
}

os::fence const& websocket_connection::fence() const {
    CTH_CRITICAL(!_state, "use after move") {}
    return _state->readyFence;
}

}
