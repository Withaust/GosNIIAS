#pragma once

#include <vector>

class Packet
{
public:
    virtual void print() = 0;
    virtual void serialize(std::vector<char>& p_out_buffer, uint16_t& p_written_bytes) = 0;
    virtual bool deserialize(std::vector<char>& p_out_buffer, uint16_t& p_written_bytes) = 0;
    virtual uint16_t get_valid_byte_count() = 0;
};
