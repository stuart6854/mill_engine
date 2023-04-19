#pragma once

#include "mill/core/base.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    class DeviceVulkan;
    class DescriptorSetLayout;
    class Buffer;

    class DescriptorSet
    {
    public:
        DescriptorSet(DeviceVulkan& device, const Shared<DescriptorSetLayout>& layout, bool buffered);
        ~DescriptorSet() = default;

        void set_uniform_buffer(u32 binding, const Shared<Buffer>& buffer);

        void flush_writes();

        /* Getters */

        auto get_set() -> vk::DescriptorSet;

    private:
        DeviceVulkan& m_device;
        Shared<DescriptorSetLayout> m_layout{};
        bool m_buffered{};

        struct Frame
        {
            vk::UniqueDescriptorSet set{};

            std::vector<vk::DescriptorBufferInfo> bufferInfos{};
            std::vector<vk::WriteDescriptorSet> pendingWrites{};
        };
        std::vector<Frame> m_frames{};
        u32 m_frameIndex{};
    };
}