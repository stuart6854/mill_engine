#include "context_vulkan.hpp"

#include "rhi_resources_vulkan.hpp"
#include "device_vulkan.hpp"
#include "image_vulkan.hpp"
#include "pipeline_vulkan.hpp"
#include "buffer_vulkan.hpp"
#include "helpers_vulkan.hpp"

namespace mill::rhi
{
    ContextVulkan::ContextVulkan(DeviceVulkan& device) : m_device(device)
    {
        vk::CommandPoolCreateInfo pool_info{};
        pool_info.setQueueFamilyIndex(m_device.get_graphics_queue_family());
        pool_info.setFlags(vk::CommandPoolCreateFlagBits::eTransient);

        vk::CommandBufferAllocateInfo alloc_info{};
        alloc_info.setCommandBufferCount(1);
        alloc_info.setLevel(vk::CommandBufferLevel::ePrimary);

        vk::SemaphoreCreateInfo semaphore_info{};
        vk::FenceCreateInfo fence_info{};

        for (auto& frame : m_frames)
        {
            frame.cmdPool = m_device.get_device().createCommandPoolUnique(pool_info);
            alloc_info.setCommandPool(frame.cmdPool.get());
            frame.cmd = std::move(m_device.get_device().allocateCommandBuffersUnique(alloc_info)[0]);
            frame.completeSemaphore = m_device.get_device().createSemaphoreUnique(semaphore_info);
            frame.fence = m_device.get_device().createFenceUnique(fence_info);
        }
    }

    void ContextVulkan::wait_and_begin()
    {
        m_associatedScreens.clear();
        m_boundPipeline = nullptr;

        m_frameIndex = (m_frameIndex + 1) % CAST_U32(m_frames.size());

        auto& frame = get_frame();

        if (frame.wasRecorded)
        {
            UNUSED(m_device.get_device().waitForFences(frame.fence.get(), true, u64_max));
            m_device.get_device().resetFences(frame.fence.get());
        }

        m_device.get_device().resetCommandPool(frame.cmdPool.get());
        frame.wasRecorded = false;

        auto& cmd = get_cmd();
        vk::CommandBufferBeginInfo begin_info{};
        cmd.begin(begin_info);
    }

    void ContextVulkan::end()
    {
        get_cmd().end();
    }

    void ContextVulkan::set_viewport(f32 x, f32 y, f32 w, f32 h, f32 min_depth, f32 max_depth)
    {
        vk::Viewport viewport{};
        viewport.setX(x);
        viewport.setY(y);
        viewport.setWidth(w);
        viewport.setHeight(h);
        viewport.setMinDepth(min_depth);
        viewport.setMaxDepth(max_depth);
        get_cmd().setViewport(0, viewport);
    }

    void ContextVulkan::set_scissor(i32 x, i32 y, u32 w, u32 h)
    {
        vk::Rect2D scissor{};
        scissor.setOffset(vk::Offset2D(x, y));
        scissor.setExtent(vk::Extent2D(w, h));
        get_cmd().setScissor(0, scissor);
    }

    void ContextVulkan::set_pipeline(HandlePipeline pipeline)
    {
        ASSERT(get_resources().pipelineMap.contains(pipeline));

        auto& pipeline_inst = get_resources().pipelineMap[pipeline];
        ASSERT(pipeline_inst->get_pipeline());

        get_cmd().bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_inst->get_pipeline());

        m_boundPipeline = pipeline_inst.get();
    }

    void ContextVulkan::set_index_buffer(HandleBuffer buffer, IndexType index_type)
    {
        ASSERT(get_resources().bufferMap.contains(buffer));

        auto& buffer_inst = get_resources().bufferMap[buffer];
        ASSERT(buffer_inst->get_buffer());
        ASSERT(buffer_inst->get_usage() == BufferUsage::eIndexBuffer);

        auto idx_type = index_type == IndexType::eU16 ? vk::IndexType::eUint16 : vk::IndexType::eUint32;
        get_cmd().bindIndexBuffer(buffer_inst->get_buffer(), 0, idx_type);
    }

    void ContextVulkan::set_vertex_buffer(HandleBuffer buffer)
    {
        ASSERT(get_resources().bufferMap.contains(buffer));

        auto& buffer_inst = get_resources().bufferMap[buffer];
        ASSERT(buffer_inst->get_buffer());
        ASSERT(buffer_inst->get_usage() == BufferUsage::eVertexBuffer);

        get_cmd().bindVertexBuffers(0, buffer_inst->get_buffer(), { 0 });
    }

    void ContextVulkan::draw(u32 vertex_count)
    {
        get_cmd().draw(vertex_count, 1, 0, 0);
    }

    void ContextVulkan::draw_indexed(u32 index_count)
    {
        get_cmd().drawIndexed(index_count, 1, 0, 0, 0);
    }

    void ContextVulkan::associate_screen(u64 screen)
    {
        m_associatedScreens.push_back(screen);
    }

    void ContextVulkan::transition_image(const vk::ImageMemoryBarrier2& barrier)
    {
        if (!barrier.image)
            return;

        vk::DependencyInfo dependency_info{};
        dependency_info.setImageMemoryBarriers(barrier);
        get_cmd().pipelineBarrier2(dependency_info);

        get_frame().wasRecorded = true;
    }

    void ContextVulkan::blit(ImageVulkan& srcImage, ImageVulkan& dstImage)
    {
        const auto src_dims = srcImage.get_dimensions();
        std::array<vk::Offset3D, 2> src_offsets = { vk::Offset3D(), vk::Offset3D(src_dims.width, src_dims.height, 1) };
        const auto dst_dims = dstImage.get_dimensions();
        std::array<vk::Offset3D, 2> dst_offsets = { vk::Offset3D(), vk::Offset3D(dst_dims.width, dst_dims.height, 1) };

        vk::ImageBlit2 region{};
        region.setSrcOffsets(src_offsets);
        region.setSrcSubresource(vulkan::get_image_subresource_layers_2d(srcImage.get_format(), 0));
        region.setDstOffsets(dst_offsets);
        region.setDstSubresource(vulkan::get_image_subresource_layers_2d(dstImage.get_format(), 0));

        vk::BlitImageInfo2 blit_info{};
        blit_info.setSrcImage(srcImage.get_image());
        blit_info.setSrcImageLayout(srcImage.get_layout());
        blit_info.setDstImage(dstImage.get_image());
        blit_info.setDstImageLayout(dstImage.get_layout());
        blit_info.setRegions(region);
        get_cmd().blitImage2(blit_info);

        get_frame().wasRecorded = true;
    }

    auto ContextVulkan::get_cmd() -> vk::CommandBuffer&
    {
        return *get_frame().cmd;
    }

    auto ContextVulkan::get_completed_semaphore() -> vk::Semaphore&
    {
        return *get_frame().completeSemaphore;
    }

    auto ContextVulkan::get_fence() -> vk::Fence&
    {
        return *get_frame().fence;
    }

    bool ContextVulkan::get_was_recorded()
    {
        return get_frame().wasRecorded;
    }

    auto ContextVulkan::get_frame() -> Frame&
    {
        return m_frames[m_frameIndex];
    }

    auto ContextVulkan::get_associated_screens() const -> const std::vector<u64>&
    {
        return m_associatedScreens;
    }

}
