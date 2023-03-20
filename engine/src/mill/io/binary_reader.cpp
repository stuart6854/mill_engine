#include "mill/io/binary_reader.hpp"

namespace mill
{
    constexpr auto open_flags = std::ios::binary;

    BinaryReader::BinaryReader(const std::string& filename) : m_filename(filename), m_stream(m_filename, open_flags) {}

    BinaryReader::BinaryReader(const std::filesystem::path& filename) : m_filename(filename), m_stream(m_filename, open_flags) {}

    BinaryReader::~BinaryReader()
    {
        m_stream.close();
    }

    void BinaryReader::skip_bytes(size_t num_bytes)
    {
        m_stream.ignore(num_bytes);
    }

    auto BinaryReader::read_i8() -> int8_t
    {
        int8_t value = {};
        m_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return value;
    }

    auto BinaryReader::read_i16() -> int16_t
    {
        int16_t value = {};
        m_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return value;
    }

    auto BinaryReader::read_i32() -> int32_t
    {
        int32_t value = {};
        m_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return value;
    }

    auto BinaryReader::read_i64() -> int64_t
    {
        int64_t value = {};
        m_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return value;
    }

    auto BinaryReader::read_u8() -> uint8_t
    {
        uint8_t value = {};
        m_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return value;
    }

    auto BinaryReader::read_u16() -> uint16_t
    {
        uint16_t value = {};
        m_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return value;
    }

    auto BinaryReader::read_u32() -> uint32_t
    {
        uint32_t value = {};
        m_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return value;
    }

    auto BinaryReader::read_u64() -> uint64_t
    {
        uint64_t value = {};
        m_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return value;
    }

    auto BinaryReader::read_f32() -> float
    {
        float value = {};
        m_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return value;
    }

    auto BinaryReader::read_f64() -> double
    {
        double value = {};
        m_stream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return value;
    }

    auto BinaryReader::read_str() -> std::string
    {
        size_t length;
        m_stream.read(reinterpret_cast<char*>(&length), sizeof(length));
        std::string buffer;
        buffer.resize(length);
        m_stream.read(&buffer[0], length);
        return buffer;
    }

}
