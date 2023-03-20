#include "mill/io/binary_writer.hpp"

#include <cassert>

namespace mill
{
    constexpr auto open_flags = std::ios::binary | std::ios::trunc;

    BinaryWriter::BinaryWriter(const std::string& filename) : m_filename(filename), m_stream(m_filename, open_flags) {}

    BinaryWriter::BinaryWriter(const std::filesystem::path& filename) : m_filename(filename), m_stream(m_filename, open_flags) {}

    BinaryWriter::~BinaryWriter()
    {
        m_stream.close();
    }

    void BinaryWriter::clear()
    {
        m_stream.close();
        m_stream = std::ofstream(m_filename, open_flags);
    }

    void BinaryWriter::write_i8(int8_t value)
    {
        m_stream.write(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void BinaryWriter::write_i16(int16_t value)
    {
        m_stream.write(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void BinaryWriter::write_i32(int32_t value)
    {
        m_stream.write(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void BinaryWriter::write_i64(int64_t value)
    {
        m_stream.write(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void BinaryWriter::write_u8(uint8_t value)
    {
        m_stream.write(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void BinaryWriter::write_u16(uint16_t value)
    {
        m_stream.write(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void BinaryWriter::write_u32(uint32_t value)
    {
        m_stream.write(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void BinaryWriter::write_u64(uint64_t value)
    {
        m_stream.write(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void BinaryWriter::write_f32(float value)
    {
        m_stream.write(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void BinaryWriter::write_f64(double value)
    {
        m_stream.write(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void BinaryWriter::write(const std::string& str)
    {
        size_t length = str.length();
        m_stream.write(reinterpret_cast<char*>(&length), sizeof(length));
        m_stream.write(&str[0], length);
    }

}