module;

#include "../io/io_log.hpp"

#include <boost/asio.hpp>
#include <cstdint>

export module network;
import cth.io.log;

import std;



namespace cth::net {
inline std::array<uint8_t, 6> parseMAC(std::string_view mac) {
    std::array<uint8_t, 6> byteMAC{};

    for(size_t i = 0, k = 0; i < 6; i++) {
        byteMAC[i] = static_cast<uint8_t>(std::stoul(std::string(mac.substr(k, 2)), nullptr, 16));
        k += 3;
    return byteMAC;
    }
}
inline std::array<uint8_t, 102> createWOLPacket(std::array<uint8_t, 6> const& byte_mac) {
    std::array<uint8_t, 102> wolPacket{};

    std::ranges::fill_n(wolPacket.begin(), 6, 0xFFui8);

    for(size_t i = 1; i <= 16; i++) std::ranges::copy(byte_mac, &wolPacket[i * 6]);
    return wolPacket;
}

//TODO create a sendUDP packet function and use it
inline void sendWOL(std::string_view target_mac, uint8_t const port = 9, std::string_view const broadcast_ip = "255.255.255.255") {
    int broadcast = 1;
    boost::asio::io_context ioContext;
    boost::asio::ip::udp::socket udpSocket(ioContext, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));


    auto const result = setsockopt(udpSocket.native_handle(), SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&broadcast), sizeof(broadcast));
    CTH_STABLE_ERR(result < 0, "failed to set broadcast option") {
        closesocket(udpSocket.native_handle());
        throw details->exception();
    }

    boost::asio::ip::udp::endpoint udpServer(boost::asio::ip::address::from_string(broadcast_ip.data()), port);

    auto const mac = parseMAC(target_mac);
    auto const wolPacket = createWOLPacket(mac);

    udpSocket.send_to(boost::asio::buffer(wolPacket), udpServer);

    closesocket(udpSocket.native_handle());
}
} // namespace cth::net
