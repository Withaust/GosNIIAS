#include "client_packet.hpp"

#include <iostream>

void ClientPacket::_parse_entry(const json& p_entry)
{
    LimitEntry entry;
    entry.name = p_entry["name"].get<std::string>();
    entry.description = p_entry["description"].get<std::string>();
    entry.bits = p_entry["bits"].get<uint8_t>();
    entry.range_min = p_entry["range_min"].get<float>();
    entry.range_max = p_entry["range_max"].get<float>();
    entry.type = p_entry["type"].get<std::string>();
    entry.word = p_entry["word"].get<uint8_t>();
    entry.offset = p_entry["offset"].get<uint8_t>();
    _entries.insert({ entry.name, entry });

    if (entry.word > _word_count)
    {
        _word_count = entry.word;
    }
}

int32_t ClientPacket::_bit_pack_float(float p_value, uint8_t p_bits, float p_range_min, float p_range_max)
{
    p_value = (p_value - p_range_min) / (p_range_max - p_range_min);
    float normalized = p_value * static_cast<float>((1 << p_bits) - 1);
    return static_cast<int32_t>(std::round(normalized));
}

float ClientPacket::_bit_unpack_float(uint32_t p_word, uint8_t p_bits, float p_range_min, float p_range_max)
{
    int32_t int_value = static_cast<int32_t>(p_word);
    float normalized = static_cast<float>(int_value) / static_cast<float>((1 << p_bits) - 1);
    return normalized * (p_range_max - p_range_min) + p_range_min;
}

void ClientPacket::_write(const LimitEntry& p_entry, float p_value)
{
    int32_t word;

    if (p_entry.type == "empty")
    {
        return;
    }
    else if (p_entry.type == "float")
    {
        word = _bit_pack_float(p_value, p_entry.bits, p_entry.range_min, p_entry.range_max);
    }
    else
    {
        word = static_cast<int32_t>(std::round(p_value));
    }

    _temp_bitset = word;

    auto& target_word = _words.at(p_entry.word);

    for (uint8_t i = 0; i < p_entry.bits; i++)
    {
        target_word[i + p_entry.offset] = _temp_bitset[i];
    }
}

float ClientPacket::_read(const LimitEntry& p_entry, bool& p_success)
{
    if (p_entry.type == "empty")
    {
        return 0.0f;
    }

    _temp_bitset.reset();

    auto& target_word = _words.at(p_entry.word);

    for (uint8_t i = 0; i < p_entry.bits; i++)
    {
        _temp_bitset[i] = target_word[i + p_entry.offset];
    }

    uint32_t word = _temp_bitset.to_ulong();

    float result;

    if (p_entry.type == "float")
    {
        result = _bit_unpack_float(word, p_entry.bits, p_entry.range_min, p_entry.range_max);
    }
    else
    {
        result = static_cast<float>(word);
    }

    if (result < p_entry.range_min || result > p_entry.range_max)
    {
        p_success = false;
    }
    else
    {
        p_success = true;
    }

    return result;
}

bool ClientPacket::_validate()
{
    for (const auto& entry : _entries)
    {
        bool success = true;
        float result = _read(entry.second, success);
        if (!success)
        {
            std::cerr << entry.second.name << " value " << result << " does not fall in range of [" <<
                entry.second.range_min << "," << entry.second.range_max << "]\n";
            return false;
        }
    }

    return true;
}

ClientPacket::ClientPacket()
{
    if (!std::filesystem::exists(_file))
    {
        throw std::runtime_error(_file + " not found");
    }

    std::ifstream f(_file, std::ios::binary);
    json data = json::parse(f);

    for (const auto& element : data)
    {
        _parse_entry(element);
    }

    // Since entries use indexes, we also need to up this by 1 to get actual count
    _word_count++;

    for (uint8_t i = 0; i < _word_count; i++)
    {
        _words.push_back({});
    }

    _byte_count = _word_count * sizeof(uint16_t);
}

ClientPacket::~ClientPacket()
{

}

const std::unordered_map<std::string, ClientPacket::LimitEntry>& ClientPacket::get_entries()
{
    return _entries;
}

void ClientPacket::write(std::string p_name, float p_value)
{
    auto itterator = _entries.find(p_name);

    if (itterator != _entries.end())
    {
        _write(itterator->second, p_value);
        return;
    }

    throw std::runtime_error("Tried writing an unknown variable " + p_name);
}

float ClientPacket::read(std::string p_name)
{
    auto itterator = _entries.find(p_name);

    if (itterator != _entries.end())
    {
        bool success = true;
        float result = _read(itterator->second, success);

        if (success)
        {
            return result;
        }
        else
        {
            throw std::runtime_error("Read variable " + std::to_string(result) + " does not fall in range of [" +
                std::to_string(itterator->second.range_min) + "," + std::to_string(itterator->second.range_max) + "]");
        }
    }

    throw std::runtime_error("Tried writing an unknown variable " + p_name);
}

template<typename T>
void format_print(T t, const int& width)
{
    std::cout << std::left << std::setw(width) << std::setfill(' ') << t;
}

size_t utf8_codepoint_count(const std::string& s)
{
    size_t count = 0;
    for (size_t i = 0; i < s.size(); )
    {
        unsigned char c = s[i];
        size_t char_len = 1;
        if ((c & 0x80) == 0) char_len = 1;
        else if ((c & 0xE0) == 0xC0) char_len = 2;
        else if ((c & 0xF0) == 0xE0) char_len = 3;
        else if ((c & 0xF8) == 0xF0) char_len = 4;
        else char_len = 1; // invalid UTF-8, fallback

        i += char_len;
        ++count;
    }
    return count;
}

template <>
void format_print<std::string>(std::string t, const int& width)
{
    size_t len = utf8_codepoint_count(t);
    std::cout << t;
    if (len < width) {
        std::cout << std::string(width - len, ' ');
    }
}

void ClientPacket::print()
{
    constexpr int width_name = 10;
    constexpr int width_desc = 20;
    constexpr int width_bits = 8;
    constexpr int width_range = 12;
    constexpr int width_type = 10;
    constexpr int width_word = 10;
    constexpr int width_offset = 10;
    constexpr int width_value = 12;

    std::cout << "============================\n";

    format_print("Name", width_name);
    format_print("Description", width_desc);
    format_print("Bits", width_bits);
    format_print("Range Min", width_range);
    format_print("Range Max", width_range);
    format_print("Type", width_type);
    format_print("Word", width_word);
    format_print("Offset", width_offset);
    format_print("Value", width_value);

    std::cout << '\n';

    for (const auto& entry : _entries)
    {
        bool success = true;
        float result = _read(entry.second, success);

        format_print(entry.second.name, width_name);
        format_print(entry.second.description, width_desc);
        format_print(static_cast<uint32_t>(entry.second.bits), width_bits);
        format_print(entry.second.range_min, width_range);
        format_print(entry.second.range_max, width_range);
        format_print(entry.second.type, width_type);
        format_print(static_cast<uint32_t>(entry.second.word), width_word);
        format_print(static_cast<uint32_t>(entry.second.offset), width_offset);
        format_print(result, width_value);

        std::cout << '\n';
    }

    std::cout << "============================\n";
}

void ClientPacket::serialize(std::vector<char>& p_out_buffer, uint16_t& p_written_bytes)
{
    if (p_out_buffer.size() < _byte_count)
    {
        p_out_buffer.resize(_byte_count, 0);
    }

    p_written_bytes = 0;

    for (uint8_t i = 0; i < _word_count; i++)
    {
        uint64_t word = _words.at(i).to_ullong();

        uint16_t word_value = static_cast<uint16_t>(word);

        std::memcpy(&p_out_buffer[p_written_bytes], &word_value, sizeof(uint16_t));

        p_written_bytes += sizeof(uint16_t);
    }
}

bool ClientPacket::deserialize(std::vector<char>& p_out_buffer, uint16_t& p_written_bytes)
{
    if (p_written_bytes != get_valid_byte_count())
    {
        return false;
    }

    uint16_t offset = 0;

    for (uint8_t i = 0; i < _word_count; i++)
    {
        uint16_t word_value;

        std::memcpy(&word_value, &p_out_buffer[offset], sizeof(uint16_t));

        auto& word = _words.at(i);

        word = word_value;

        offset += sizeof(uint16_t);
    }

    return _validate();
}

uint16_t ClientPacket::get_valid_byte_count()
{
    return _byte_count;
}
