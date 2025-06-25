#pragma once

#define _WIN32_WINNT 0x0601
#include "thirdparty/asio/include/asio.hpp"

#include "client_packet.hpp"
#include "server_packet.hpp"

class Client
{
private:

    asio::ip::udp::socket _socket;
    asio::ip::udp::endpoint _target_endpoint;
    asio::ip::udp::endpoint _sender_endpoint;
    ClientPacket _client_packet;
    ServerPacket _server_packet;
    std::vector<char> _message;
    uint16_t _written_bytes = 0;

    void _send();
    void _handle_send(std::error_code p_error_code, std::size_t p_bytes_sent);
    void _receive();
    void _handle_recieve(std::error_code p_error_code, std::size_t p_bytes_recieved);

public:

    Client(asio::io_context& p_io_context,
        const std::string& p_bind_ip, unsigned short p_bind_port,
        const std::string& p_target_ip, unsigned short p_target_port);

};
