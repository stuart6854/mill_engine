#pragma once

#if MILL_WINDOWS
    #define VK_USE_PLATFORM_WIN32_KHR
#endif
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
// #include <vk_mem_alloc.h>
#include "vk_mem_alloc/vk_mem_alloc.hpp"

namespace mill::platform::vulkan
{
    constexpr u32 g_FrameBufferCount = 2;
}