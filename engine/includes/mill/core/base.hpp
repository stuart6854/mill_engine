#pragma once

#include "debug.hpp"

#include <cstdint>
#include <memory>

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