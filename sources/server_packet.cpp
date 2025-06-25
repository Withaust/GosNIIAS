#include "server_packet.hpp"

#include <iostream>

ServerPacket::ServerPacket()
{

}

ServerPacket::~ServerPacket()
{

}

void ServerPacket::set_identified(bool p_value)
{
    _bitset[0] = p_value;
}

bool ServerPacket::is_identified()
{
    return _bitset[0] == false;
}

void ServerPacket::set_correct(bool p_value)
{
    _bitset[1] = p_value;
}

bool ServerPacket::is_correct()
{
    return _bitset[1] == true;
}

void ServerPacket::print()
{
    std::cout << "============================\n";

    std::cout << "is_identified: " << is_identified() << " is_correct: " << is_correct() << "\n";

    std::cout << "============================\n";
}

void ServerPacket::serialize(std::vector<char>& p_out_buffer, uint16_t& p_written_bytes)
{
    p_out_buffer[0] = static_cast<int8_t>(_bitset.to_ulong());

    p_written_bytes = 1;
}

bool ServerPacket::deserialize(std::vector<char>& p_out_buffer, uint16_t& p_written_bytes)
{
    if (p_written_bytes != get_valid_byte_count())
    {
        return false;
    }

    uint32_t value = static_cast<uint32_t>(p_out_buffer[0]);
    _bitset = value;

    return true;
}

uint16_t ServerPacket::get_valid_byte_count()
{
    return 1;
}
