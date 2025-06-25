#include "client.hpp"

#include <iostream>

void Client::_send()
{
    for (const auto& entry : _client_packet.get_entries())
    {
        if (entry.second.type == "empty")
        {
            continue;
        }

        float value;

        std::cout << "Please enter a value for a variable \"" << entry.first << "\" PREFERABLY within a range of "
            << "[" << entry.second.range_min << "," << entry.second.range_max << "] (not enforced)\n";

        while (!(std::cin >> value))
        {
            std::cin.clear(); // Clear error flags
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
            std::cout << "Invalid input. Please enter a valid value:\n";
        }

        _client_packet.write(entry.first, value);
    }

    // Sending example test packet
    /*
    _client_packet.write("X", 20);
    _client_packet.write("Y", 30);
    _client_packet.write("V", 40);
    _client_packet.write("M", 1);
    _client_packet.write("S", 2);
    _client_packet.write("A", 12.0);
    _client_packet.write("P", 130); // Try setting this to something like 140 and see that error checking&correction kicks in as expected
    */
    _client_packet.serialize(_message, _written_bytes);

    _socket.async_send_to(asio::buffer(_message, _written_bytes), _target_endpoint,
        std::bind(&Client::_handle_send, this, std::placeholders::_1, std::placeholders::_2));
}

void Client::_handle_send(std::error_code p_error_code, std::size_t p_bytes_sent)
{
    if (!p_error_code)
    {
        std::cout << "Sent client packet to " << _target_endpoint << "!\n";
        _client_packet.print();
        _receive();
    }
    else
    {
        std::cerr << "Send failed: " << p_error_code.message() << "\n";
    }
}

void Client::_receive()
{
    _socket.async_receive_from(asio::buffer(_message), _sender_endpoint,
        std::bind(&Client::_handle_recieve, this, std::placeholders::_1, std::placeholders::_2));
}

void Client::_handle_recieve(std::error_code p_error_code, std::size_t p_bytes_recieved)
{
    if (p_error_code)
    {
        std::cerr << "Receive failed: " << p_error_code.message() << "\n";
        return;
    }

    if (p_bytes_recieved == _server_packet.get_valid_byte_count())
    {
        _written_bytes = static_cast<uint16_t>(p_bytes_recieved);
        bool result = _server_packet.deserialize(_message, _written_bytes);

        if (result)
        {
            std::cout << "Received server packet from " << _sender_endpoint << "!\n";
            _server_packet.print();
        }
    }
    else
    {
        std::cerr << "Receive failed due to incorrect packet size of " << p_bytes_recieved << " being different from expected " << _server_packet.get_valid_byte_count() << "\n";
    }
}

Client::Client(asio::io_context& p_io_context,
    const std::string& p_bind_ip, unsigned short p_bind_port,
    const std::string& p_target_ip, unsigned short p_target_port)
    : _socket(p_io_context, asio::ip::udp::endpoint(asio::ip::make_address(p_bind_ip), p_bind_port)),
    _target_endpoint(asio::ip::make_address(p_target_ip), p_target_port),
    _message(1024)
{
    _send();
}
