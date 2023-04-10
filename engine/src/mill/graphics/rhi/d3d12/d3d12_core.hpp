#pragma once

#include "mill/core/debug.hpp"

namespace mill::platform
{
    template <typename T>
    constexpr void safe_release(T*& resource)
    {
        if (resource)
        {
            resource->Release();
            resource = nullptr;
        }
    }

    inline void assert_if_failed(HRESULT hr)
    {
        ASSERT(SUCCEEDED(hr));
    }

    template<typename T>
    constexpr void name_object(T* obj, LPCWSTR name)
    {
        obj->SetName(name);
    }

    inline uint32_t AlignU32(uint32_t valueToAlign, uint32_t alignment)
    {
        alignment -= 1;
        return (uint32_t)((valueToAlign + alignment) & ~alignment);
    }
}