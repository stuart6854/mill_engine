#include "rhi_resource_set_vulkan.hpp"

#include "../rhi_core_vulkan.hpp"
#include "../vulkan_device.hpp"
#include "../vulkan_helpers.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    auto create_resource_set(const ResourceSetDescription& description) -> u64
    {
        auto& device = get_device();

        const auto vk_desc = to_vulkan(description);
        const auto set_id = device.create_resource_set(vk_desc);
        ASSERT(set_id);

        return set_id;
    }

    auto to_vulkan(const ResourceBinding& binding) -> ResourceBindingVulkan
    {
        ResourceBindingVulkan out_binding{};
        out_binding.type = to_vulkan(binding.type);
        out_binding.count = binding.count;
        out_binding.shaderStages = to_vulkan(binding.shaderStages);
        return out_binding;
    }

    auto to_vulkan(const ResourceSetDescription& input_desc) -> ResourceSetDescriptionVulkan
    {
        ResourceSetDescriptionVulkan out_desc{};

        out_desc.bindings.resize(input_desc.bindings.size());
        for (u32 i = 0; i < out_desc.bindings.size(); ++i)
            out_desc.bindings[i] = to_vulkan(input_desc.bindings[i]);

        out_desc.buffered = input_desc.buffer;

        return out_desc;
    }
}
