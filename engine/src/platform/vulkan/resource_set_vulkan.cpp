#include "resource_set_vulkan.hpp"

#include "device_vulkan.hpp"

namespace mill::rhi
{
    ResourceSetVulkan::ResourceSetVulkan(DeviceVulkan& device, vk::DescriptorSetLayout layout) : m_device(device)
    {
        std::vector<vk::DescriptorSetLayout> set_layouts(g_FrameBufferCount, layout);

        vk::DescriptorSetAllocateInfo alloc_info{};
        alloc_info.setDescriptorSetCount(g_FrameBufferCount);
        alloc_info.setSetLayouts(set_layouts);
        alloc_info.setDescriptorPool(m_device.get_descriptor_pool());
        auto sets = m_device.get_device().allocateDescriptorSetsUnique(alloc_info);

        for (auto i = 0; i < g_FrameBufferCount; ++i)
        {
            m_sets[i] = std::move(sets[i]);
        }
    }

    ResourceSetVulkan::~ResourceSetVulkan() = default;

    void ResourceSetVulkan::bind_buffer(u32 binding, BufferVulkan& buffer)
    {
        UNUSED(binding);
        UNUSED(buffer);
    }

}
