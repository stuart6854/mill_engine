#pragma once

#include "mill/core/base.hpp"
#include "mill/graphics/rhi/resources/rhi_pipeline.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    auto to_vulkan(BufferUsage usage) -> vk::BufferUsageFlags;
    auto to_vulkan(MemoryUsage mem_usage) -> vma::MemoryUsage;

    struct BufferDescriptionVulkan
    {
        u64 size{};
        vk::BufferUsageFlags usage{};
        vma::MemoryUsage memoryUsage{};
        vma::AllocationCreateFlags allocFlags{};
    };
    auto to_vulkan(const BufferDescription& in_desc) -> BufferDescriptionVulkan;
}
