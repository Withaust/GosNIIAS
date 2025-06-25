#pragma once

#include <bitset>

#include "packet.hpp"

class ServerPacket : public Packet
{
private:

    std::bitset<sizeof(uint8_t) * CHAR_BIT> _bitset;

public:

    ServerPacket();
    virtual ~ServerPacket();

    void set_identified(bool p_value);
    bool is_identified();
    void set_correct(bool p_value);
    bool is_correct();

    void print() override;
    void serialize(std::vector<char>& p_out_buffer, uint16_t& p_written_bytes) override;
    bool deserialize(std::vector<char>& p_out_buffer, uint16_t& p_written_bytes) override;
    uint16_t get_valid_byte_count() override;
};

