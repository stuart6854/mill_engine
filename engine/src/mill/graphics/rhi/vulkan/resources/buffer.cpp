#include "buffer.hpp"

#include "mill/core/base.hpp"
#include "../vulkan_device.hpp"
#include "../vulkan_includes.hpp"

#include <tuple>

namespace mill::rhi
{
    Buffer::Buffer(DeviceVulkan& device) : m_device(device) {}

    Buffer::~Buffer() = default;

    void Buffer::set_size(u64 size)
    {
        m_size = size;
    }

    void Buffer::set_usage(vk::BufferUsageFlags usage)
    {
        m_usage = usage;
    }

    void Buffer::set_memory_usage(vma::MemoryUsage mem_usage)
    {
        m_memUsage = mem_usage;
    }

    void Buffer::set_alloc_flags(vma::AllocationCreateFlags flags)
    {
        m_allocFlags = flags;
    }

    void Buffer::build()
    {
        auto& allocator = m_device.get_allocator();

        vk::BufferCreateInfo buffer_info{};
        buffer_info.setSize(m_size);
        buffer_info.setUsage(m_usage);

        vma::AllocationCreateInfo alloc_info{};
        alloc_info.setUsage(m_memUsage);
        alloc_info.setFlags(m_allocFlags);

        std::tie(m_buffer, m_allocation) = allocator.createBufferUnique(buffer_info, alloc_info);
    }

    auto Buffer::get_size() const -> u64
    {
        return m_size;
    }

    auto Buffer::get_usage() const -> vk::BufferUsageFlags
    {
        return m_usage;
    }

    auto Buffer::get_memory_usage() const -> vma::MemoryUsage
    {
        return m_memUsage;
    }

    auto Buffer::get_alloc_flags() const -> vma::AllocationCreateFlags
    {
        return m_allocFlags;
    }

    auto Buffer::get_buffer() const -> const vk::Buffer&
    {
        return m_buffer.get();
    }

    auto Buffer::get_allocation() const -> const vma::Allocation&
    {
        return m_allocation.get();
    }

}
