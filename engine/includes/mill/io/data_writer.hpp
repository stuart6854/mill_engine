#pragma once

#include <cstdint>
#include <string>

namespace mill
{
    /**
     * @brief Interface for defining a data writer.
     */
    class DataWriter
    {
    public:
        virtual ~DataWriter() = default;

        /* Commands */

        virtual void clear() = 0;

        virtual void write_i8(int8_t value) = 0;
        virtual void write_i16(int16_t value) = 0;
        virtual void write_i32(int32_t value) = 0;
        virtual void write_i64(int64_t value) = 0;

        virtual void write_u8(uint8_t value) = 0;
        virtual void write_u16(uint16_t value) = 0;
        virtual void write_u32(uint32_t value) = 0;
        virtual void write_u64(uint64_t value) = 0;

        virtual void write_f32(float value) = 0;
        virtual void write_f64(double value) = 0;

        virtual void write(const std::string& str) = 0;
    };

    /**
     * @brief DummyWriter does not actually write any data anywhere. It is useful for calculating the size of data before actually writing
     * it out.
     */
    class DummyWriter final : public DataWriter
    {
    public:
        DummyWriter() = default;
        ~DummyWriter() = default;

        void clear() override;

        void write_i8(int8_t value) override;
        void write_i16(int16_t value) override;
        void write_i32(int32_t value) override;
        void write_i64(int64_t value) override;

        void write_u8(uint8_t value) override;
        void write_u16(uint16_t value) override;
        void write_u32(uint32_t value) override;
        void write_u64(uint64_t value) override;

        void write_f32(float value) override;
        void write_f64(double value) override;

        void write(const std::string& str) override;

        auto get_data_size() const -> size_t;

    private:
        size_t m_dataSize = 0;
    };
}