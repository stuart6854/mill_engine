#pragma once

#include "debug.hpp"

#include <cstdint>
#include <memory>
#include <functional>

namespace mill
{
    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    constexpr u32 u32_max = UINT32_MAX;
    constexpr u64 u64_max = UINT64_MAX;

    using byte = uint8_t;
    using sizet = size_t;

    using f32 = float;
    using f64 = double;

    using hasht = u64;

    template <typename T>
    inline void hash_combine(hasht& seed, const T& value)
    {
        std::hash<T> h;
        seed ^= h(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    inline u64 operator""_hs(const char* c_str, sizet length)
    {
        std::string_view str(c_str, length);
        return std::hash<std::string_view>{}(str);
    }

    template <typename T>
    inline auto vec_data_size(const std::vector<T>& vec) -> sizet
    {
        return sizeof(T) * vec.size();
    }

    template <typename T>
    using Shared = std::shared_ptr<T>;

    template <typename T, typename... Args>
    constexpr Shared<T> CreateShared(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    using Owned = std::unique_ptr<T>;

    template <typename T, typename... Args>
    constexpr Owned<T> CreateOwned(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
}

#ifndef UNUSED
    #define UNUSED(_var) (void)_var
#endif

#ifndef CAST_U32
    #define CAST_U8(_x) static_cast<u8>(_x)
    #define CAST_U16(_x) static_cast<u16>(_x)
    #define CAST_U32(_x) static_cast<u32>(_x)
    #define CAST_U64(_x) static_cast<u64>(_x)
#endif

#ifndef DISABLE_COPY
    #define DISABLE_COPY(_type) \
        _type(_type&) = delete; \
        auto operator=(_type&)->_type& = delete
#endif

#ifndef DISABLE_MOVE
    #define DISABLE_MOVE(_type)  \
        _type(_type&&) = delete; \
        auto operator=(_type&&)->_type& = delete
#endif

#ifndef DISABLE_COPY_AND_MOVE
    #define DISABLE_COPY_AND_MOVE(_type) \
        DISABLE_COPY(_type);             \
        DISABLE_MOVE(_type)
#endif