#include "context_vk.hpp"

#include "device_vk.hpp"

namespace mill::platform::vulkan
{
    ContextVulkan::ContextVulkan(DeviceVulkan& device) : m_device(device)
    {
        auto vk_device = m_device.get_device();

        vk::CommandPoolCreateInfo pool_info{};
        pool_info.setQueueFamilyIndex(m_device.get_graphics_queue_family());
        pool_info.setFlags(vk::CommandPoolCreateFlagBits::eTransient);

        vk::CommandBufferAllocateInfo cmd_alloc_info{};
        cmd_alloc_info.setCommandBufferCount(1);
        cmd_alloc_info.setLevel(vk::CommandBufferLevel::ePrimary);

        vk::SemaphoreCreateInfo semaphore_info{};
        vk::FenceCreateInfo fence_info{ vk::FenceCreateFlagBits::eSignaled };

        for (auto& frame : m_frames)
        {
            frame.cmdPool = vk_device.createCommandPool(pool_info);

            cmd_alloc_info.setCommandPool(frame.cmdPool);
            frame.cmdBuffer = vk_device.allocateCommandBuffers(cmd_alloc_info)[0];

            frame.semaphore = vk_device.createSemaphore(semaphore_info);
            frame.fence = vk_device.createFence(fence_info);
        }
    }

    ContextVulkan::~ContextVulkan()
    {
        for (auto& frame : m_frames)
        {
            Receipt receipt{ {}, frame.fence };
            m_device.wait_for(receipt);
        }

        auto vk_device = m_device.get_device();

        for (auto& frame : m_frames)
        {
            vk_device.destroy(frame.cmdPool);
            vk_device.destroy(frame.semaphore);
            vk_device.destroy(frame.fence);
            frame = {};
        }
    }

    void ContextVulkan::begin_frame()
    {
        m_frameIndex = (m_frameIndex + 1) % g_FrameBufferCount;

        auto& frame = m_frames[m_frameIndex];

        UNUSED(m_device.get_device().waitForFences(frame.fence, true, u64_max));
        m_device.get_device().resetFences(frame.fence);

        m_device.get_device().resetCommandPool(frame.cmdPool);

        vk::CommandBufferBeginInfo begin_info{};
        frame.cmdBuffer.begin(begin_info);

        m_boundPipeline = nullptr;
    }

    void ContextVulkan::end_frame()
    {
        get_current_cmd().end();
    }

    void ContextVulkan::add_barrier(ImageVulkan& image_resource, vk::ImageLayout new_layout)
    {
        auto old_layout = image_resource.layout;

        vk::PipelineStageFlags2 srcStage{};
        vk::AccessFlags2 srcAccess{};
        vk::PipelineStageFlags2 dstStage{};
        vk::AccessFlags2 dstAccess{};
        if (new_layout == vk::ImageLayout::eAttachmentOptimal)
        {
            if (old_layout == vk::ImageLayout::eUndefined || old_layout == vk::ImageLayout::ePresentSrcKHR)
            {
                srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            }

            dstStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            dstAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
        }
        else if (new_layout == vk::ImageLayout::ePresentSrcKHR)
        {
            if (old_layout == vk::ImageLayout::eUndefined || old_layout == vk::ImageLayout::ePresentSrcKHR)
            {
                srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
                srcAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
            }
        }

        vk::ImageMemoryBarrier2 image_barrier{};
        image_barrier.setSrcStageMask(srcStage);
        image_barrier.setDstStageMask(dstStage);
        image_barrier.setOldLayout(old_layout);
        image_barrier.setNewLayout(new_layout);
        image_barrier.setImage(image_resource.image);
        image_barrier.setSubresourceRange(image_resource.range);

        vk::DependencyInfo dep_info{};
        dep_info.setImageMemoryBarriers(image_barrier);
        get_current_cmd().pipelineBarrier2(dep_info);

        image_resource.layout = new_layout;
    }

    void ContextVulkan::begin_render_pass(ImageVulkan& image_resource, const glm::vec4* clear_color)
    {
        vk::RenderingAttachmentInfo attachment_info{};
        attachment_info.setImageLayout(image_resource.layout);
        attachment_info.setImageView(image_resource.view);
        attachment_info.setLoadOp(clear_color == nullptr ? vk::AttachmentLoadOp::eDontCare : vk::AttachmentLoadOp::eClear);
        attachment_info.setStoreOp(vk::AttachmentStoreOp::eStore);

        std::array<f32, 4> clear_value{};
        if (clear_color != nullptr)
        {
            clear_value = { clear_color->x, clear_color->y, clear_color->z, clear_color->w };
            attachment_info.setClearValue(vk::ClearColorValue(clear_value));
        }

        vk::RenderingInfo rendering_info{};
        rendering_info.setColorAttachments(attachment_info);
        rendering_info.setLayerCount(1);
        rendering_info.setRenderArea(vk::Rect2D({ 0, 0 }, image_resource.extent));

        get_current_cmd().beginRendering(rendering_info);
    }

    void ContextVulkan::end_render_pass()
    {
        get_current_cmd().endRendering();
    }

    void ContextVulkan::set_pipeline(PipelineVulkan& pipeline)
    {
        get_current_cmd().bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline);
        m_boundPipeline = &pipeline;
    }

    void ContextVulkan::set_viewport(const vk::Viewport& viewport)
    {
        get_current_cmd().setViewport(0, viewport);
    }

    void ContextVulkan::set_scissor(const vk::Rect2D& scissor)
    {
        get_current_cmd().setScissor(0, scissor);
    }

    void ContextVulkan::set_default_viewport_and_scissor(const glm::uvec2& screen_size)
    {
        vk::Viewport viewport{};
        viewport.setX(0);
        viewport.setY(0);
        viewport.setWidth(static_cast<f32>(screen_size.x));
        viewport.setHeight(static_cast<f32>(screen_size.y));
        viewport.setMinDepth(0.0f);
        viewport.setMaxDepth(1.0f);
        set_viewport(viewport);

        vk::Rect2D scissor{};
        scissor.setOffset({ 0, 0 });
        scissor.setExtent({ screen_size.x, screen_size.y });
        set_scissor(scissor);
    }

    void ContextVulkan::set_index_buffer(BufferVulkan& index_buffer, vk::IndexType index_type)
    {
        ASSERT(index_buffer.buffer && index_buffer.allocation);
        ASSERT(index_buffer.usage & vk::BufferUsageFlagBits::eIndexBuffer);

        get_current_cmd().bindIndexBuffer(index_buffer.buffer, { 0 }, index_type);
        m_boundIndexBuffer = &index_buffer;
    }

    void ContextVulkan::set_vertex_buffer(BufferVulkan& vertex_buffer)
    {
        ASSERT(vertex_buffer.buffer && vertex_buffer.allocation);
        ASSERT(vertex_buffer.usage & vk::BufferUsageFlagBits::eVertexBuffer);

        get_current_cmd().bindVertexBuffers(0, vertex_buffer.buffer, { 0 });
        m_boundVertexBuffer = &vertex_buffer;
    }

    void ContextVulkan::draw(u32 vertex_count, u32 vertex_offset)
    {
        ASSERT(m_boundIndexBuffer != nullptr);
        if (!m_boundIndexBuffer->is_ready)
        {
            LOG_DEBUG("ContextVulkan - Index buffer is not ready.");
            return;
        }

        get_current_cmd().draw(vertex_count, 1, vertex_offset, 0);
    }

    void ContextVulkan::draw_indexed(u32 index_count, u32 index_offset, u32 vertex_offset)
    {
        if (!m_boundIndexBuffer->is_ready || !m_boundVertexBuffer->is_ready)
        {
            LOG_DEBUG("ContextVulkan - Index/Vertex buffers are not ready.");
            return;
        }

        get_current_cmd().drawIndexed(index_count, 1, index_offset, vertex_offset, 0);
    }

    auto ContextVulkan::get_current_cmd() const -> vk::CommandBuffer
    {
        return m_frames[m_frameIndex].cmdBuffer;
    }

    auto ContextVulkan::get_current_semaphore() const -> vk::Semaphore
    {
        return m_frames[m_frameIndex].semaphore;
    }

    auto ContextVulkan::get_current_fence() const -> vk::Fence
    {
        return m_frames[m_frameIndex].fence;
    }

}
