#include "rhi_resource_set_vulkan.hpp"

#include "../rhi_core_vulkan.hpp"
#include "descriptor_set.hpp"
#include "../vulkan_device.hpp"
#include "../vulkan_view.hpp"
#include "../vulkan_image.hpp"
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

    void bind_buffer_to_resource_set(u64 resource_set_id, u32 binding, u64 buffer_id)
    {
        auto& device = get_device();

        const auto& buffer = device.get_buffer(buffer_id);

        auto& resource_set = device.get_resource_set(resource_set_id);

        LOG_DEBUG("RHI Vulkan - Binding buffer <{}> to resource set <{}> in binding {}.", buffer_id, resource_set_id, binding);
        resource_set->set_uniform_buffer(binding, buffer);
    }

    void bind_texture_to_resource_set(u64 resource_set_id, u32 binding, u64 texture_id)
    {
        auto& device = get_device();

        const auto& texture = device.get_texture(texture_id);

        auto& resource_set = device.get_resource_set(resource_set_id);

        LOG_DEBUG("RHI Vulkan - Binding texture <{}> to resource set <{}> in binding {}.", texture_id, resource_set_id, binding);
        resource_set->set_image(binding, texture);
    }

    void bind_view_to_resource_set(u64 resource_set_id, u32 binding, u64 view_id)
    {
        auto& device = get_device();

        auto* view = device.get_view(view_id);
        ASSERT(view);

        auto* view_color_image = view->get_color_attachment();
        if (view_color_image->get_sampler() == nullptr)
        {
            SamplerDescriptionVulkan sampler_desc{
                .filter = vk::Filter::eLinear,
            };
            auto sampler = device.get_or_create_sampler(sampler_desc);
            view_color_image->set_sampler(sampler);
        }

        auto& resource_set = device.get_resource_set(resource_set_id);

        LOG_DEBUG("RHI Vulkan - Binding view <{}> to resource set <{}> in binding {}.", view_id, resource_set_id, binding);
        resource_set->set_image(binding, *view_color_image);
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
