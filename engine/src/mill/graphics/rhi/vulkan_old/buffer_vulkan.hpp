#pragma once

#include "mill/core/base.hpp"
#include "mill/graphics/rhi.hpp"
#include "includes_vulkan.hpp"

namespace mill::rhi
{
    class InstanceVulkan;

    class BufferVulkan
    {
    public:
        BufferVulkan(InstanceVulkan& device, const BufferDescription& description);
        ~BufferVulkan();

        void write(u64 offset, u64 size, const void* data);

        /* Getters */

        auto get_size() const -> u64;
        auto get_usage() const -> BufferUsage;
        auto get_mem_usage() const -> MemoryUsage;

        auto get_buffer() const -> const vk::Buffer&;

    private:
        InstanceVulkan& m_device;
        BufferDescription m_desc{};

        vma::UniqueBuffer m_buffer{};
        vma::UniqueAllocation m_allocation{};
    };
}