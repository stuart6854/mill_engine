#pragma once

#include "mill/core/base.hpp"
#include "includes_vulkan.hpp"

namespace mill::rhi
{
    class ContextVulkan;

    class ImageVulkan
    {
    public:
        ImageVulkan(class InstanceVulkan& device, vk::Image image, vk::ImageUsageFlags usage, vk::Extent3D dimensions, vk::Format format);
        ImageVulkan(
            class InstanceVulkan& device, vk::ImageUsageFlags usage, vk::Extent3D dimensions, vk::Format format, u32 mip_levels = 1);
        ~ImageVulkan();

        void transition_to_transfer_src(ContextVulkan& context);
        void transition_to_transfer_dst(ContextVulkan& context);
        void transition_to_attachment(ContextVulkan& context);
        void transition_to_present(ContextVulkan& context);

        void set_layout(vk::ImageLayout layout);

        /* Getters */

        auto get_image() -> vk::Image&;
        auto get_view() -> vk::ImageView&;
        auto get_layout() -> vk::ImageLayout&;

        auto get_usage() -> vk::ImageUsageFlags;
        auto get_dimensions() -> vk::Extent3D;
        auto get_format() -> vk::Format;
        auto get_mip_levels() -> u32;

    private:
        InstanceVulkan& m_device;

        vk::ImageUsageFlags m_usage{};
        vk::Extent3D m_dimensions{};
        vk::Format m_format{};
        u32 m_mipLevels{};

        vk::Image m_image{};
        vma::Allocation m_allocation{};
        vk::ImageView m_view{};

        vk::ImageLayout m_layout{ vk::ImageLayout::eUndefined };
    };
}
