#pragma once

#include "mill/graphics/rhi/resources/rhi_texture.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    struct TextureDescriptionVulkan
    {
        vk::Extent3D extent{};
        vk::Format format{};
        u32 mipLevels{};
    };

    auto to_vulkan(const TextureDescription& in_desc) -> TextureDescriptionVulkan;
}