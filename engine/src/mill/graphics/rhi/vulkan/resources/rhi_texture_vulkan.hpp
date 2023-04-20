#pragma once

#include "mill/graphics/rhi/resources/rhi_texture.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    struct SamplerDescriptionVulkan
    {
        vk::Filter filter{};
    };

    struct TextureDescriptionVulkan
    {
        vk::Extent3D extent{};
        vk::Format format{};
        u32 mipLevels{};

        SamplerDescriptionVulkan samplerDesc{};
    };

    auto to_vulkan(FilterMode filter_mode) -> vk::Filter;
    auto to_vulkan(const TextureDescription& in_desc) -> TextureDescriptionVulkan;
}