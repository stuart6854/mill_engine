#include "buffer_vulkan.hpp"

#include "device_vulkan.hpp"

namespace mill::rhi
{
    namespace
    {
        auto to_vulkan(BufferUsage usage) -> vk::BufferUsageFlags
        {
            switch (usage)
            {
                case BufferUsage::eVertexBuffer: return vk::BufferUsageFlagBits::eVertexBuffer;
                case BufferUsage::eIndexBuffer: return vk::BufferUsageFlagBits::eIndexBuffer;
                case BufferUsage::eUniformBuffer: return vk::BufferUsageFlagBits::eUniformBuffer;
                case BufferUsage::eTransferSrc: return vk::BufferUsageFlagBits::eTransferSrc;
                default:
                    LOG_ERROR("RHI - Vulkan - Unknown BufferUsage!");
                    ASSERT(false);
                    break;
            }
            return {};
        }

        auto to_vma(MemoryUsage usage) -> vma::MemoryUsage
        {
            switch (usage)
            {
                case mill::rhi::MemoryUsage::eHost: return vma::MemoryUsage::eAutoPreferHost;
                case mill::rhi::MemoryUsage::eDevice: return vma::MemoryUsage::eAutoPreferDevice;
                case mill::rhi::MemoryUsage::eDeviceHostVisble: return vma::MemoryUsage::eAutoPreferDevice;
                default:
                    LOG_ERROR("RHI - Vulkan - Unknown MemoryUsage!");
                    ASSERT(false);
                    break;
            }
            return {};
        }
    }

    BufferVulkan::BufferVulkan(DeviceVulkan& device, const BufferDescription& description) : m_device(device), m_desc(description)
    {
        ASSERT(m_desc.size);

        vk::BufferCreateInfo buffer_info{};
        buffer_info.setSize(m_desc.size);
        buffer_info.setUsage(to_vulkan(m_desc.usage));

        vma::AllocationCreateInfo alloc_info{};
        alloc_info.setUsage(to_vma(m_desc.memoryUsage));
        if (m_desc.memoryUsage == MemoryUsage::eDeviceHostVisble)
            alloc_info.setFlags(vma::AllocationCreateFlagBits::eHostAccessSequentialWrite);

        std::tie(m_buffer, m_allocation) = m_device.get_allocator().createBufferUnique(buffer_info, alloc_info);
    }

    BufferVulkan::~BufferVulkan() = default;

    void BufferVulkan::write(u64 offset, u64 size, const void* data)
    {
        ASSERT(size);
        ASSERT(offset + size <= m_desc.size);
        ASSERT(m_desc.memoryUsage == MemoryUsage::eHost || m_desc.memoryUsage == MemoryUsage::eDeviceHostVisble);

        void* mapped_dst = m_device.get_allocator().mapMemory(m_allocation.get());
        u8* offset_dst = static_cast<u8*>(mapped_dst) + offset;
        std::memcpy(offset_dst, data, size);
        m_device.get_allocator().unmapMemory(m_allocation.get());
    }

    auto BufferVulkan::get_size() const -> u64
    {
        return m_desc.size;
    }

    auto BufferVulkan::get_usage() const -> BufferUsage
    {
        return m_desc.usage;
    }

    auto BufferVulkan::get_mem_usage() const -> MemoryUsage
    {
        return m_desc.memoryUsage;
    }

    auto BufferVulkan::get_buffer() const -> const vk::Buffer&
    {
        return m_buffer.get();
    }

}
