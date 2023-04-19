#pragma once

#include "mill/graphics/rhi/resources/rhi_resource_set.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    struct ResourceBindingVulkan
    {
        vk::DescriptorType type{};
        u32 count{};
        vk::ShaderStageFlags shaderStages{};
    };
    auto to_vulkan(const ResourceBinding& binding) -> ResourceBindingVulkan;

    struct ResourceSetDescriptionVulkan
    {
        std::vector<ResourceBindingVulkan> bindings{};
        bool buffered{};
    };
    auto to_vulkan(const ResourceSetDescription& input_desc) -> ResourceSetDescriptionVulkan;
}