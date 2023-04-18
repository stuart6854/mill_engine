#pragma once

#include "mill/core/base.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    class DeviceVulkan;

    class Buffer
    {
    public:
        Buffer(DeviceVulkan& device);
        ~Buffer();

        void set_size(u64 size);
        void set_usage(vk::BufferUsageFlags usage);
        void set_memory_usage(vma::MemoryUsage mem_usage);
        void set_alloc_flags(vma::AllocationCreateFlags flags);

        void build();

        /* Getters */

        auto get_size() const -> u64;
        auto get_usage() const -> vk::BufferUsageFlags;
        auto get_memory_usage() const -> vma::MemoryUsage;
        auto get_alloc_flags() const -> vma::AllocationCreateFlags;

        auto get_buffer() const -> const vk::Buffer&;
        auto get_allocation() const -> const vma::Allocation&;

    private:
        DeviceVulkan& m_device;

        u64 m_size{};
        vk::BufferUsageFlags m_usage{};
        vma::MemoryUsage m_memUsage{};
        vma::AllocationCreateFlags m_allocFlags{};

        vma::UniqueBuffer m_buffer{};
        vma::UniqueAllocation m_allocation{};
    };
}