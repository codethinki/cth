#include "test.hpp"

#include "cth/net/coro/websocket.hpp"
#include "cth/net/websocket_server.hpp"

#include <cth/coro/executor.hpp>
#include <cth/coro/sync.hpp>
#include <cth/coro/tasks/executor_task.hpp>

#include <hv/WebSocketClient.h>

#include <array>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <format>
#include <future>
#include <mutex>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace cth::net {
namespace {
struct client_message {
    std::vector<std::byte> bytes;
    bool text;
};

struct client_t {
    hv::WebSocketClient socket;
    std::mutex mutex;
    std::condition_variable changed;
    std::deque<client_message> messages;
    bool opened = false;
    bool closed = false;

    explicit client_t(std::uint16_t port) {
        socket.setPingInterval(0);
        socket.onopen = [this] {
            {
                std::scoped_lock const _{mutex};
                opened = true;
            }
            changed.notify_all();
        };
        socket.onmessage = [this](std::string const& data) {
            {
                std::scoped_lock const _{mutex};
                messages.emplace_back(
                    std::vector<std::byte>{
                        reinterpret_cast<std::byte const*>(data.data()),
                        reinterpret_cast<std::byte const*>(data.data() + data.size())
                    },
                    socket.opcode() == WS_OPCODE_TEXT
                );
            }
            changed.notify_all();
        };
        socket.onclose = [this] {
            {
                std::scoped_lock const _{mutex};
                closed = true;
            }
            changed.notify_all();
        };

        auto const url = std::format("ws://127.0.0.1:{}/", port);
        if(socket.open(url.c_str()) != 0)
            throw std::runtime_error{"failed to open WebSocket test client"};

        std::unique_lock lock{mutex};
        if(!changed.wait_for(lock, std::chrono::seconds{2}, [this] { return opened || closed; }) ||
            !opened)
            throw std::runtime_error{"WebSocket test client did not connect"};
    }

    ~client_t() { socket.stop(); }

    void send_text(std::string_view message) {
        if(socket.send(std::string{message}) < 0)
            throw std::runtime_error{"failed to send WebSocket text message"};
    }

    void send_bytes(std::span<std::byte const> message) {
        if(socket.send(
            reinterpret_cast<char const*>(message.data()),
            static_cast<int>(message.size()),
            WS_OPCODE_BINARY
        ) < 0)
            throw std::runtime_error{"failed to send WebSocket binary message"};
    }

    client_message receive() {
        std::unique_lock lock{mutex};
        if(!changed.wait_for(
            lock,
            std::chrono::seconds{2},
            [this] { return !messages.empty() || closed; }
        ) || messages.empty())
            throw std::runtime_error{"WebSocket test client did not receive a message"};

        auto message = std::move(messages.front());
        messages.pop_front();
        return message;
    }
};
}

NET_TEST(websocket_server, receives_text_and_binary_messages) {
    websocket_server server;
    auto received = std::async(std::launch::async, [&server] {
        auto connection = server.accept();
        EXPECT_TRUE(connection);
        EXPECT_TRUE(connection->wait());
        auto const textType = connection->got_text();
        auto text = connection->read_text();
        EXPECT_TRUE(connection->wait());
        auto const binaryType = connection->got_text();
        auto binary = connection->read_bytes();
        return std::tuple{std::move(text), textType, std::move(binary), binaryType};
    });

    auto const port = server.port();
    ASSERT_TRUE(port);
    client_t client{*port};
    client.send_text("hello");
    std::array binary{std::byte{1}, std::byte{2}, std::byte{3}};
    client.send_bytes(binary);

    auto [text, textType, bytes, binaryType] = received.get();
    EXPECT_EQ(text, "hello");
    EXPECT_TRUE(textType);
    EXPECT_EQ(bytes, std::vector(binary.begin(), binary.end()));
    EXPECT_FALSE(binaryType);
}

NET_TEST(websocket_server, sends_text_and_binary_messages) {
    websocket_server server;
    auto sent = std::async(std::launch::async, [&server] {
        auto connection = server.accept();
        EXPECT_TRUE(connection);
        connection->write_text("hello");
        std::array binary{std::byte{1}, std::byte{2}, std::byte{3}};
        connection->write_bytes(binary);
    });

    auto const port = server.port();
    ASSERT_TRUE(port);
    client_t client{*port};
    auto text = client.receive();
    auto binary = client.receive();
    sent.get();

    EXPECT_TRUE(text.text);
    EXPECT_EQ(
        std::string(reinterpret_cast<char const*>(text.bytes.data()), text.bytes.size()),
        "hello"
    );
    EXPECT_FALSE(binary.text);
    EXPECT_EQ(binary.bytes, (std::vector{std::byte{1}, std::byte{2}, std::byte{3}}));
}

NET_TEST(websocket_server, coroutine_facade_receives_and_sends) {
    co::websocket_server server;
    auto const port = server.port();
    ASSERT_TRUE(port);

    auto client = std::async(std::launch::async, [port = *port] {
        client_t result{port};
        std::array request{std::byte{4}, std::byte{5}};
        result.send_bytes(request);
        return result.receive().bytes;
    });

    auto exchange = [&server]() -> cth::co::executor_task<std::vector<std::byte>> {
        auto connection = co_await server.accept();
        if(!connection)
            co_return std::vector<std::byte>{};

        auto request = co_await connection->read_bytes();
        connection->write_bytes(request);
        co_return request;
    };

    cth::co::scheduler scheduler{cth::co::autostart, 2};
    auto request = cth::co::sync(cth::co::executor{scheduler}.spawn(exchange()));
    auto response = client.get();

    EXPECT_EQ(request, (std::vector{std::byte{4}, std::byte{5}}));
    EXPECT_EQ(response, (std::vector{std::byte{4}, std::byte{5}}));
}

NET_TEST(websocket_server, coroutine_accept_does_not_block_scheduler) {
    co::websocket_server server;
    auto const port = server.port();
    ASSERT_TRUE(port);

    cth::co::scheduler scheduler{cth::co::autostart, 1};
    cth::co::executor executor{scheduler};
    auto accept = [&server]() -> cth::co::executor_task<bool> {
        co_return (co_await server.accept()).has_value();
    };
    auto accepted = executor.spawn(accept());

    std::promise<void> marker;
    auto marked = marker.get_future();
    scheduler.post([&marker] { marker.set_value(); });

    EXPECT_EQ(marked.wait_for(std::chrono::seconds{1}), std::future_status::ready);
    client_t client{*port};
    EXPECT_TRUE(cth::co::sync(std::move(accepted)));
}

NET_TEST(websocket_server, stop_interrupts_accept) {
    websocket_server server;
    auto accepted = std::async(std::launch::async, [&server] { return server.accept(); });

    server.stop();

    EXPECT_EQ(accepted.wait_for(std::chrono::seconds{1}), std::future_status::ready);
    EXPECT_FALSE(accepted.get().has_value());
}

NET_TEST(websocket_server, accept_times_out) {
    websocket_server server;

    EXPECT_FALSE(server.accept(std::chrono::milliseconds{5}).has_value());
    EXPECT_FALSE(server.stopped());
}

NET_TEST(websocket_server, stop_interrupts_wait) {
    websocket_server server;
    auto accepted = std::async(std::launch::async, [&server] { return server.accept(); });
    auto const port = server.port();
    ASSERT_TRUE(port);
    client_t client{*port};
    auto connection = accepted.get();
    ASSERT_TRUE(connection);

    auto received = std::async(std::launch::async, [&connection] { return connection->wait(); });
    server.stop();

    EXPECT_EQ(received.wait_for(std::chrono::seconds{1}), std::future_status::ready);
    EXPECT_FALSE(received.get());
}

NET_TEST(websocket_server, movable) {
    static_assert(std::is_nothrow_move_constructible_v<websocket_server>);
    static_assert(std::is_nothrow_move_assignable_v<websocket_server>);

    websocket_server source;
    auto const port = source.port();
    ASSERT_TRUE(port);

    websocket_server server{std::move(source)};
    auto accepted = std::async(std::launch::async, [&server] { return server.accept(); });
    client_t client{*port};

    EXPECT_TRUE(accepted.get().has_value());
}

}
