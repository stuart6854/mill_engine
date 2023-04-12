#include "vulkan_context.hpp"

#include "mill/core/base.hpp"
#include "mill/core/debug.hpp"
#include "rhi_core_vulkan.hpp"
#include "vulkan_device.hpp"
#include "vulkan_image.hpp"
#include "vulkan_helpers.hpp"

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

    ContextVulkan::~ContextVulkan() = default;

    void ContextVulkan::wait_and_begin()
    {
        m_frameIndex = (m_frameIndex + 1) % CAST_U32(m_frames.size());

        auto& frame = get_frame();

        if (frame.wasRecorded)
        {
            UNUSED(m_device.get_device().waitForFences(frame.fence.get(), true, u64_max));
            m_device.get_device().resetFences(frame.fence.get());
        }

        m_device.get_device().resetCommandPool(frame.cmdPool.get());
        frame.wasRecorded = false;

        m_associatedScreenIds.clear();
        m_pipelineState = {};
        m_compiledPipelineStateHash = 0;
        m_pipelineStateDirty = false;

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

        get_frame().wasRecorded = true;
    }

    void ContextVulkan::set_scissor(i32 x, i32 y, u32 w, u32 h)
    {
        vk::Rect2D scissor{};
        scissor.setOffset(vk::Offset2D(x, y));
        scissor.setExtent(vk::Extent2D(w, h));
        get_cmd().setScissor(0, scissor);

        get_frame().wasRecorded = true;
    }

    void ContextVulkan::set_pipeline_vertex_input_state(hasht state_hash)
    {
        m_pipelineState.vertexInputStateHash = state_hash;
        m_pipelineStateDirty = true;
    }

    void ContextVulkan::set_pipeline_pre_rasterisation_state(hasht state_hash)
    {
        m_pipelineState.preRasterisationStateHash = state_hash;
        m_pipelineStateDirty = true;
    }

    void ContextVulkan::set_pipeline_fragment_stage_state(hasht state_hash)
    {
        m_pipelineState.fragmentStageStateHash = state_hash;
        m_pipelineStateDirty = true;
    }

    void ContextVulkan::set_pipeline_fragment_output_state(hasht state_hash)
    {
        m_pipelineState.fragmentOutputStateHash = state_hash;
        m_pipelineStateDirty = true;
    }

    void ContextVulkan::draw(u32 vertex_count)
    {
        verify_pipeline_state();

        get_cmd().draw(vertex_count, 1, 0, 0);

        get_frame().wasRecorded = true;
    }

    void ContextVulkan::transition_image(ImageVulkan& image, vk::ImageLayout new_layout)
    {
        const auto old_layout = image.get_layout();
        if (new_layout == old_layout)
            return;

        const auto subresource_range = vulkan::get_image_subresource_range_2d(image.get_format());
        const auto src_stage_mask = vulkan::get_image_layout_stage_mask(old_layout);
        const auto dst_stage_mask = vulkan::get_image_layout_stage_mask(new_layout);
        const auto src_access_mask = vulkan::get_image_layout_access_mask(old_layout);
        const auto dst_access_mask = vulkan::get_image_layout_access_mask(new_layout);

        vk::ImageMemoryBarrier2 barrier{};
        barrier.setImage(image.get_image());
        barrier.setSubresourceRange(subresource_range);
        barrier.setOldLayout(old_layout);
        barrier.setNewLayout(new_layout);
        barrier.setSrcStageMask(src_stage_mask);
        barrier.setSrcAccessMask(src_access_mask);
        barrier.setDstStageMask(dst_stage_mask);
        barrier.setDstAccessMask(dst_access_mask);

        vk::DependencyInfo dependency{};
        dependency.setImageMemoryBarriers(barrier);

        get_cmd().pipelineBarrier2(dependency);

        image.set_layout(new_layout);

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

    void ContextVulkan::associate_screen(u64 screen_id)
    {
        m_associatedScreenIds.push_back(screen_id);
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

    auto ContextVulkan::get_associated_screen_ids() const -> const std::vector<u64>&
    {
        return m_associatedScreenIds;
    }

    void ContextVulkan::verify_pipeline_state()
    {
        if (!m_pipelineStateDirty)
            return;

        m_pipelineStateDirty = false;

        const auto new_pipeline_state_hash = m_pipelineState.get_hash();
        if (new_pipeline_state_hash == m_compiledPipelineStateHash)
            return;

        auto& device = get_device();

        if (!device.is_pipeline_compiled(m_pipelineState))
        {
            device.compile_pipeline(m_pipelineState);
        }
        m_compiledPipelineStateHash = new_pipeline_state_hash;

        auto& pipeline = device.get_pipeline(m_compiledPipelineStateHash);
        get_cmd().bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    }

}
