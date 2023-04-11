#pragma once

#include "mill/core/base.hpp"
#include "rhi_core_vulkan.hpp"
#include "includes_vulkan.hpp"

namespace mill::rhi
{
    class InstanceVulkan;
    class BufferVulkan;

    class ResourceSetVulkan
    {
    public:
        ResourceSetVulkan(InstanceVulkan& device, vk::DescriptorSetLayout layout);
        ~ResourceSetVulkan();

        void bind_buffer(u32 binding, BufferVulkan& buffer);

    private:
        InstanceVulkan& m_device;

        std::array<vk::UniqueDescriptorSet, g_FrameBufferCount> m_sets{};
        u32 m_frameIndex{};
    };
}