#pragma once

#include "mill/core/base.hpp"
#include "common_headers_vk.hpp"
#include "helpers_vk.hpp"

namespace mill::platform::vulkan
{
    struct ImageInit
    {
        u32 width{};
        u32 height{};
        u32 depth{ 1 };
        vk::ImageUsageFlags usage{};
        vk::Format format{};
    };

    struct ImageVulkan
    {
        bool is_ready{ false };

        u64 id{};
        ImageInit init{};

        vk::Image image{};
        vma::Allocation allocation{};
        vk::ImageView view{};
        vk::Extent3D extent{};
        vk::ImageLayout layout{ vk::ImageLayout::eUndefined };
        vk::ImageSubresourceRange range{};
        u32 boundIndex{ 0 };
    };

    struct PipelineState
    {
        vk::PrimitiveTopology topology{ vk::PrimitiveTopology::eTriangleList };

        // Rasterization
        vk::PolygonMode polygonMode{ vk::PolygonMode::eFill };  // Use this for wireframe
        vk::FrontFace frontFace{ vk::FrontFace::eCounterClockwise };
        vk::CullModeFlags cullMode{ vk::CullModeFlagBits::eBack };

        vk::Format attachmentFormat{};
    };

    struct PipelineInit
    {
        std::vector<u32> vertexSrc{};
        std::vector<u32> fragmentSrc{};
        PipelineLayout* layout{ nullptr };
        PipelineState state{};
    };

    struct PipelineVulkan
    {
        vk::PipelineLayout layout{};
        vk::Pipeline pipeline{};
        PipelineState state{};
        u64 stateHash{};
    };

    struct BufferInit
    {
        u64 size{};
        vk::BufferUsageFlags usage{};
        bool isCPUVisible{ false };
        const void* initial_data = nullptr;
    };

    struct BufferVulkan
    {
        bool is_ready{ false };

        u64 id{};
        u64 size{};
        vk::BufferUsageFlags usage{};
        bool isCPUVisible{ false };

        vk::Buffer buffer{};
        vma::Allocation allocation{};
        void* mappedPtr{ nullptr };

        void write(u64 offset_bytes, u64 size_bytes, const void* data)
        {
            ASSERT(isCPUVisible && mappedPtr != nullptr);
            ASSERT(size_bytes <= size);
            ASSERT(offset_bytes + size_bytes <= size);

            u8* offset_ptr = static_cast<u8*>(mappedPtr) + offset_bytes;
            std::memcpy(offset_ptr, data, size_bytes);
        }
    };
}
