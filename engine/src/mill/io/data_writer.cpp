#include "mill/io/data_writer.hpp"

namespace mill
{
    void DummyWriter::clear()
    {
        m_dataSize = 0;
    }

    void DummyWriter::write_i8(int8_t)
    {
        m_dataSize += sizeof(int8_t);
    }

    void DummyWriter::write_i16(int16_t)
    {
        m_dataSize += sizeof(int16_t);
    }

    void DummyWriter::write_i32(int32_t)
    {
        m_dataSize += sizeof(int32_t);
    }

    void DummyWriter::write_i64(int64_t)
    {
        m_dataSize += sizeof(int64_t);
    }

    void DummyWriter::write_u8(uint8_t)
    {
        m_dataSize += sizeof(uint8_t);
    }

    void DummyWriter::write_u16(uint16_t)
    {
        m_dataSize += sizeof(uint16_t);
    }

    void DummyWriter::write_u32(uint32_t)
    {
        m_dataSize += sizeof(uint32_t);
    }

    void DummyWriter::write_u64(uint64_t)
    {
        m_dataSize += sizeof(uint64_t);
    }

    void DummyWriter::write_f32(float)
    {
        m_dataSize += sizeof(float);
    }

    void DummyWriter::write_f64(double)
    {
        m_dataSize += sizeof(double);
    }

    void DummyWriter::write(const std::string& str)
    {
        m_dataSize += sizeof(size_t);
        m_dataSize += str.size();
    }

    auto DummyWriter::get_data_size() const -> size_t
    {
        return m_dataSize;
    }
}
