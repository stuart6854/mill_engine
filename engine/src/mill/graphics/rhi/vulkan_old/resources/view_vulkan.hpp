#pragma once

#include "mill/core/base.hpp"
#include "../includes_vulkan.hpp"

namespace mill::rhi
{
    class ImageVulkan;

    class ViewVulkan
    {
    public:
        ViewVulkan(class InstanceVulkan& device);
        ~ViewVulkan();

        void reset(u32 width, u32 height);

        void begin(vk::CommandBuffer& cmd);
        void end(vk::CommandBuffer& cmd);

        void set_clear_color(const vk::ClearColorValue& clear_value);
        void set_clear_depth_stencil(const vk::ClearDepthStencilValue& clear_value);

        /* Getters */
        auto get_color_attachment() -> ImageVulkan*;
        auto get_depth_attachment() -> ImageVulkan*;

    private:
        InstanceVulkan& m_device;
        vk::Extent2D m_resolution{};

        Owned<ImageVulkan> m_colorImage{};
        Owned<ImageVulkan> m_depthImage{};

        vk::RenderingAttachmentInfo m_colorAttachment{};
        vk::RenderingAttachmentInfo m_depthAttachment{};
        vk::RenderingInfo m_renderingInfo{};
    };
}
