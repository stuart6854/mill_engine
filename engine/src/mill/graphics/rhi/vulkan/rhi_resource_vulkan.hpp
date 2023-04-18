#pragma once

#include "mill/core/base.hpp"
#include "resources/rhi_pipeline_vulkan.hpp"
#include "vulkan_includes.hpp"

namespace mill::rhi
{
    auto to_vulkan(ResourceType type) -> vk::DescriptorType;

    auto to_vulkan(ShaderStageFlags stages) -> vk::ShaderStageFlags;


    auto to_vulkan(Format format) -> vk::Format;
}
