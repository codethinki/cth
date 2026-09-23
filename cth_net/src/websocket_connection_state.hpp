#pragma once

#include <cth/os/fence.hpp>

#include <hv/WebSocketChannel.h>

#include <cstddef>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

namespace cth::net::dev {

struct websocket_message {
    std::vector<std::byte> bytes;
    bool text;
};

struct websocket_connection_state {
    WebSocketChannelPtr channel;
    std::size_t maxMessageSize;

    mutable std::mutex mutex;
    std::deque<websocket_message> messages;
    os::fence readyFence;
    bool closed = false;

    websocket_connection_state(WebSocketChannelPtr channel, std::size_t maxMessageSize);

    void receive(WebSocketChannelPtr const& source, std::string const& data);
    void close(bool closeChannel) noexcept;
};

}
