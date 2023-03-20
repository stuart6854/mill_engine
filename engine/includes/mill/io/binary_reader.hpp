#pragma once

#include "data_reader.hpp"

#include <string>
#include <fstream>
#include <ios>
#include <filesystem>

namespace mill
{
    /**
     * @brief Read binary data from disk.
     */
    class BinaryReader : public DataReader
    {
    public:
        explicit BinaryReader(const std::string& filename);
        explicit BinaryReader(const std::filesystem::path& filename);
        ~BinaryReader();

        void skip_bytes(size_t num_bytes) override;

        auto read_i8() -> int8_t override;
        auto read_i16() -> int16_t override;
        auto read_i32() -> int32_t override;
        auto read_i64() -> int64_t override;

        auto read_u8() -> uint8_t override;
        auto read_u16() -> uint16_t override;
        auto read_u32() -> uint32_t override;
        auto read_u64() -> uint64_t override;

        auto read_f32() -> float override;
        auto read_f64() -> double override;

        auto read_str() -> std::string override;

    private:
        std::filesystem::path m_filename;
        std::ifstream m_stream;
    };

}