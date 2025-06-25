#pragma once

#define _WIN32_WINNT 0x0601
#include "thirdparty/asio/include/asio.hpp"

#include "client_packet.hpp"
#include "server_packet.hpp"

class Server
{
private:

    asio::ip::udp::socket _socket;
    asio::ip::udp::endpoint _remote_endpoint;
    std::array<char, 1024> _recv_buffer;
    ClientPacket _client_packet;
    ServerPacket _server_packet;
    std::vector<char> _message;
    uint16_t _written_bytes = 0;

    void _receive();
    void _handle_receive(const std::error_code& p_error_code, std::size_t p_bytes_recieved);
    void _handle_send(const std::error_code& p_error_code, std::size_t p_bytes_sent);

public:

    Server(asio::io_context& p_io_context, const std::string& p_bind_ip, unsigned short p_port);
};
