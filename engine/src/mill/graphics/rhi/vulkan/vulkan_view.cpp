#include "vulkan_view.hpp"

#include "mill/core/base.hpp"
#include "mill/core/debug.hpp"
#include "vulkan_device.hpp"
#include "vulkan_image.hpp"

namespace mill::rhi
{
    ViewVulkan::ViewVulkan(DeviceVulkan& device) : m_device(device) {}

    ViewVulkan::~ViewVulkan() = default;

    void ViewVulkan::reset(u32 width, u32 height)
    {
        if (m_resolution.width == width && m_resolution.height == height)
        {
            return;
        }

        m_device.wait_idle();

        m_colorImage.reset();
        m_depthImage.reset();

        m_resolution = vk::Extent2D(width, height);

        m_colorImage = CreateOwned<ImageVulkan>(m_device,
                                                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
                                                vk::Extent3D(m_resolution, 1),
                                                vk::Format::eR8G8B8A8Unorm);
        m_colorAttachment.setImageLayout(vk::ImageLayout::eAttachmentOptimal);
        m_colorAttachment.setImageView(m_colorImage->get_view());
        m_colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
        m_colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);

        m_depthImage = CreateOwned<ImageVulkan>(
            m_device, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::Extent3D(m_resolution, 1), vk::Format::eD24UnormS8Uint);
        m_depthAttachment.setImageLayout(vk::ImageLayout::eAttachmentOptimal);
        m_depthAttachment.setImageView(m_depthImage->get_view());
        m_depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
        m_depthAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);

        m_renderingInfo.setColorAttachments(m_colorAttachment);
        m_renderingInfo.setPDepthAttachment(&m_depthAttachment);
        m_renderingInfo.setLayerCount(1);
        m_renderingInfo.setRenderArea(vk::Rect2D({ 0, 0 }, m_resolution));
    }

    void ViewVulkan::begin(vk::CommandBuffer& cmd)
    {
        cmd.beginRendering(m_renderingInfo);
    }

    void ViewVulkan::end(vk::CommandBuffer& cmd)
    {
        cmd.endRendering();
    }

    void ViewVulkan::set_clear_color(const vk::ClearColorValue& clear_value)
    {
        m_colorAttachment.setClearValue(clear_value);
    }

    void ViewVulkan::set_clear_depth_stencil(const vk::ClearDepthStencilValue& clear_value)
    {
        m_depthAttachment.setClearValue(clear_value);
    }

    auto ViewVulkan::get_color_attachment() -> ImageVulkan*
    {
        return m_colorImage.get();
    }

    auto ViewVulkan::get_depth_attachment() -> ImageVulkan*
    {
        return m_depthImage.get();
    }

}
