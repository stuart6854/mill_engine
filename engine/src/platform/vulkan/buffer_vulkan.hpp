#pragma once

#include "mill/core/base.hpp"
#include "mill/platform/rhi.hpp"
#include "includes_vulkan.hpp"

namespace mill::rhi
{
    class DeviceVulkan;

    class BufferVulkan
    {
    public:
        BufferVulkan(DeviceVulkan& device, const BufferDescription& description);
        ~BufferVulkan();

        /* Getters */

        auto get_size() const -> u64;
        auto get_usage() const -> BufferUsage;
        auto get_buffer() const -> const vk::Buffer&;

    private:
        DeviceVulkan& m_device;
        BufferDescription m_desc{};

        vma::UniqueBuffer m_buffer{};
        vma::UniqueAllocation m_allocation{};
    };
}