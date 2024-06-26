#pragma once
#include <array>
#include <iostream>
#include <string>

#if defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
#error "WinSock.h" included but "WinSock2.h" is required (maybe you included Windows.h before this?)
#endif
#include <boost/asio.hpp>

//TODO write tests for this



namespace cth::net {
inline std::array<uint8_t, 6> parseMAC(const std::string_view mac) {
    std::array<uint8_t, 6> byteMAC{};

    for(size_t i = 0, k = 0; i < 6; i++) {
        byteMAC[i] = static_cast<uint8_t>(std::stoul(std::string(mac.substr(k, 2)), nullptr, 16));
        k += 3;
    }
    return byteMAC;
}
inline std::array<uint8_t, 102> createWOLPacket(const std::array<uint8_t, 6>& byte_mac) {
    std::array<uint8_t, 102> wolPacket{};

    std::ranges::fill_n(wolPacket.begin(), 6, 0xFFui8);

    for(size_t i = 1; i <= 16; i++) std::ranges::copy(byte_mac, &wolPacket[i * 6]);
    return wolPacket;
}

//TODO create a sendUDP packet function and use it
inline void sendWOL(const std::string_view target_mac, const uint8_t port = 9, const std::string_view broadcast_ip = "255.255.255.255") {
    int broadcast = 1;
    boost::asio::io_context ioContext;
    boost::asio::ip::udp::socket udpSocket(ioContext, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));

    if(setsockopt(udpSocket.native_handle(), SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&broadcast), sizeof(broadcast)) < 0) {
        std::cerr << "Error in setting Broadcast option.\n";
        closesocket(udpSocket.native_handle());
        return;
    }

    boost::asio::ip::udp::endpoint udpServer(boost::asio::ip::address::from_string(broadcast_ip.data()), port);

    const auto mac = parseMAC(target_mac);
    const auto wolPacket = createWOLPacket(mac);

    udpSocket.send_to(boost::asio::buffer(wolPacket), udpServer);

    closesocket(udpSocket.native_handle());
}
} // namespace cth::net
