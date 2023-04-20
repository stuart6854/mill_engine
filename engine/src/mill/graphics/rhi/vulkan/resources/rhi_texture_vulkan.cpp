#include "rhi_texture_vulkan.hpp"

#include "../rhi_core_vulkan.hpp"
#include "../vulkan_device.hpp"
#include "../vulkan_helpers.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    auto to_vulkan(FilterMode filter_mode) -> vk::Filter
    {
        switch (filter_mode)
        {
            case mill::rhi::FilterMode::eLinear: return vk::Filter::eLinear;
            case mill::rhi::FilterMode::eNearest: return vk::Filter::eNearest;
            default: ASSERT(("Unknown FilterMode!", false)); break;
        }
        return {};
    }

    auto to_vulkan(const TextureDescription& in_desc) -> TextureDescriptionVulkan
    {
        TextureDescriptionVulkan out_desc{};
        out_desc.extent = vk::Extent3D(in_desc.dimensions.x, in_desc.dimensions.y, in_desc.dimensions.z);
        out_desc.format = to_vulkan(in_desc.format);
        out_desc.mipLevels = in_desc.mipLevels;
        out_desc.samplerDesc.filter = to_vulkan(in_desc.filterMode);
        return out_desc;
    }

    auto create_texture(const TextureDescription& description) -> u64
    {
        auto& device = get_device();

        const auto vk_desc = to_vulkan(description);
        const auto id = device.create_texture(vk_desc);
        return id;
    }

    void write_texture(u64 texture_id, u32 mip_level, const void* data)
    {
        auto& device = get_device();
        device.write_texture(texture_id, mip_level, data);
    }

    void generate_mip_maps(u64 texture_id)
    {
        auto& device = get_device();
        device.generate_mip_maps(texture_id);
    }
}
