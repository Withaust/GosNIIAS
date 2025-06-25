#include "server.hpp"

#include <iostream>

using asio::ip::udp;

void Server::_receive()
{
    _socket.async_receive_from(asio::buffer(_message), _remote_endpoint,
        std::bind(&Server::_handle_receive, this, std::placeholders::_1, std::placeholders::_2));
}

void Server::_handle_receive(const std::error_code& p_error_code, std::size_t p_bytes_recieved)
{
    if (p_error_code)
    {
        std::cerr << "Send failed: " << p_error_code.message() << "\n";
    }
    else
    {
        if (p_bytes_recieved == _client_packet.get_valid_byte_count())
        {
            std::cout << "Received client packet from " << _remote_endpoint << "!\n";
            _written_bytes = static_cast<uint16_t>(p_bytes_recieved);
            bool success = _client_packet.deserialize(_message, _written_bytes);
            _client_packet.print();

            _server_packet.set_identified(true);
            _server_packet.set_correct(success);
            _server_packet.serialize(_message, _written_bytes);

            _socket.async_send_to(asio::buffer(_message, _written_bytes), _remote_endpoint,
                std::bind(&Server::_handle_send, this, std::placeholders::_1, std::placeholders::_2));
        }
        else
        {
            std::cerr << "Receive failed due to incorrect packet size of " << p_bytes_recieved << " being different from expected " << _server_packet.get_valid_byte_count() << "\n";
        }
    }
    _receive();
}

void Server::_handle_send(const std::error_code& p_error_code, std::size_t p_bytes_sent)
{
    if (!p_error_code)
    {
        std::cout << "Sent server packet to " << _remote_endpoint << "!\n";
        _server_packet.print();
    }
    else
    {
        std::cerr << "Send failed: " << p_error_code.message() << "\n";
    }
}

Server::Server(asio::io_context& io_context, const std::string& bind_ip, unsigned short port)
    : _socket(io_context, asio::ip::udp::endpoint(asio::ip::make_address(bind_ip), port)),
    _message(1024)
{
    _receive();
}
