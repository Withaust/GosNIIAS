#pragma once

#include <filesystem>
#include <fstream>
#include <bitset>
#include "thirdparty/nlohmann/include/json.hpp"
using json = nlohmann::json;

#include "packet.hpp"

class ClientPacket : public Packet
{
public:

    struct LimitEntry
    {
        std::string name;
        std::string description;
        uint8_t bits = 0;
        float range_min = 0.0f;
        float range_max = 0.0f;
        std::string type;
        uint8_t word = 0;
        uint8_t offset = 0;
    };

private:

    std::string _file = "limits.json";
    std::unordered_map<std::string, LimitEntry> _entries;
    uint8_t _word_count = 0;
    uint16_t _byte_count = 0;
    std::vector<std::bitset<sizeof(uint16_t)* CHAR_BIT>> _words;
    std::bitset<sizeof(uint16_t)* CHAR_BIT> _temp_bitset;

    void _parse_entry(const json& p_entry);
    int32_t _bit_pack_float(float p_value, uint8_t p_bits, float p_range_min, float p_range_max);
    float _bit_unpack_float(uint32_t p_word, uint8_t p_bits, float p_range_min, float p_range_max);
    void _write(const LimitEntry& p_entry, float p_value);
    // DOES NOT THROW on invalid limits, used internally by _validate due to non-throwing behaviour
    float _read(const LimitEntry& p_entry, bool& p_success);
    bool _validate();

public:

    ClientPacket();
    virtual ~ClientPacket();

    const std::unordered_map<std::string, LimitEntry>& get_entries();

    void write(std::string p_name, float p_value);
    // THROWS on invalid limits since this is an exposed API
    float read(std::string p_name);
    void print() override;
    void serialize(std::vector<char>& p_out_buffer, uint16_t& p_written_bytes) override;
    bool deserialize(std::vector<char>& p_out_buffer, uint16_t& p_written_bytes) override;
    uint16_t get_valid_byte_count() override;
};

