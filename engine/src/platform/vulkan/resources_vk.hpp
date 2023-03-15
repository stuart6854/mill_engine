#pragma once

#include "mill/core/base.hpp"
#include "common_headers_vk.hpp"

namespace mill::platform::vulkan
{
    struct ImageVulkan
    {
        vk::Image image{};
        vk::ImageView view{};
        vk::Extent2D extent{};
        vk::ImageLayout layout{};
        vk::ImageSubresourceRange range{};
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
        std::vector<u32> vertexSrc;
        std::vector<u32> fragmentSrc;

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
        void* initial_data = nullptr;
    };

    struct BufferVulkan
    {
        u64 id{};
        u64 size{};
        vk::BufferUsageFlags usage{};
        bool isCPUVisible{ false };

        vk::Buffer buffer{};
        VmaAllocation allocation{};
    };
}