#pragma once

#include "mill/core/base.hpp"
#include "rhi_core_vulkan.hpp"
#include "vulkan_includes.hpp"

namespace mill::rhi
{
    class DeviceVulkan;
    class ImageVulkan;

    class ContextVulkan
    {
    public:
        ContextVulkan(class DeviceVulkan& device);
        ~ContextVulkan();

        void wait_and_begin();
        void end();

#if 0
        void set_viewport(f32 x, f32 y, f32 w, f32 h, f32 min_depth, f32 max_depth);
        void set_scissor(i32 x, i32 y, u32 w, u32 h);

        void set_index_buffer(HandleBuffer buffer, IndexType index_type);
        void set_vertex_buffer(HandleBuffer buffer);

        void draw(u32 vertex_count);
        void draw_indexed(u32 index_count);

#endif
        void transition_image(ImageVulkan& image, vk::ImageLayout new_layout);

        void blit(ImageVulkan& srcImage, ImageVulkan& dstImage);

        void associate_screen(u64 screen_id);

        /* Getters */

        auto get_cmd() -> vk::CommandBuffer&;
        auto get_completed_semaphore() -> vk::Semaphore&;
        auto get_fence() -> vk::Fence&;
        bool get_was_recorded();

        auto get_associated_screen_ids() const -> const std::vector<u64>&;

    private:
        struct Frame;
        auto get_frame() -> Frame&;

    private:
        DeviceVulkan& m_device;

        struct Frame
        {
            vk::UniqueCommandPool cmdPool{};
            vk::UniqueCommandBuffer cmd{};
            bool wasRecorded{ false };

            vk::UniqueSemaphore completeSemaphore{};
            vk::UniqueFence fence{};
        };
        std::array<Frame, g_FrameBufferCount> m_frames{};
        u32 m_frameIndex{};

        std::vector<u64> m_associatedScreenIds{};
        // vk::Pipeline m_boundPipeline{};
    };
}
