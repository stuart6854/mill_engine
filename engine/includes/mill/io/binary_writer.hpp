#pragma once

#include "data_writer.hpp"

#include <string>
#include <fstream>
#include <filesystem>

namespace mill
{
    /**
     * @brief Write binary data to disk.
     */
    class BinaryWriter final : public DataWriter
    {
    public:
        explicit BinaryWriter(const std::string& filename);
        explicit BinaryWriter(const std::filesystem::path& filename);
        ~BinaryWriter();

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

    private:
        std::filesystem::path m_filename;
        std::ofstream m_stream;
    };

}