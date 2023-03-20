#pragma once

#include <cstdint>
#include <string>

namespace mill
{
    /**
     * @brief Interface for defining a data reader.
     */
    class DataReader
    {
    public:
        virtual ~DataReader() = default;

        /* Commands */

        virtual void skip_bytes(size_t num_bytes) = 0;

        virtual auto read_i8() -> int8_t = 0;
        virtual auto read_i16() -> int16_t = 0;
        virtual auto read_i32() -> int32_t = 0;
        virtual auto read_i64() -> int64_t = 0;

        virtual auto read_u8() -> uint8_t = 0;
        virtual auto read_u16() -> uint16_t = 0;
        virtual auto read_u32() -> uint32_t = 0;
        virtual auto read_u64() -> uint64_t = 0;

        virtual auto read_f32() -> float = 0;
        virtual auto read_f64() -> double = 0;

        virtual auto read_str() -> std::string = 0;
    };
}