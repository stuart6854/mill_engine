#include "rhi_buffer_vulkan.hpp"

#include "../rhi_core_vulkan.hpp"
#include "../rhi_resource_vulkan.hpp"

#include "../vulkan_device.hpp"
#include "../vulkan_helpers.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    auto to_vulkan(BufferUsage usage) -> vk::BufferUsageFlags
    {
        switch (usage)
        {
            case BufferUsage::eVertexBuffer: return vk::BufferUsageFlagBits::eVertexBuffer;
            case BufferUsage::eIndexBuffer: return vk::BufferUsageFlagBits::eIndexBuffer;
            case BufferUsage::eUniformBuffer: return vk::BufferUsageFlagBits::eUniformBuffer;
            case BufferUsage::eTransferSrc: return vk::BufferUsageFlagBits::eTransferSrc;
            default: ASSERT(("Unknown BufferUsage!", false)); break;
        }
        return {};
    }

    auto to_vulkan(MemoryUsage mem_usage) -> vma::MemoryUsage
    {
        switch (mem_usage)
        {
            case MemoryUsage::eHost: return vma::MemoryUsage::eAutoPreferHost;
            case MemoryUsage::eDevice:
            case MemoryUsage::eDeviceHostVisble: return vma::MemoryUsage::eAutoPreferHost;
            default: ASSERT(("Unknown MemoryUsage!", false)); break;
        }
        return {};
    }

    auto to_vulkan(const BufferDescription& in_desc) -> BufferDescriptionVulkan
    {
        BufferDescriptionVulkan out_desc{};
        out_desc.size = in_desc.size;
        out_desc.usage = to_vulkan(in_desc.usage);
        out_desc.memoryUsage = to_vulkan(in_desc.memoryUsage);
        if (in_desc.memoryUsage == MemoryUsage::eDeviceHostVisble)
            out_desc.allocFlags |= vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;
        return out_desc;
    }

    auto create_buffer(const BufferDescription& description) -> HandleBuffer
    {
        auto& device = get_device();

        const auto buffer_desc = to_vulkan(description);
        return device.create_buffer(buffer_desc);
    }

    void write_buffer(HandleBuffer /*buffer*/, u64 /*offset*/, u64 /*size*/, const void* /*data*/) {}

}