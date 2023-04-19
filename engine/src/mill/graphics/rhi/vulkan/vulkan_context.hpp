#pragma once

#include "mill/core/base.hpp"
#include "rhi_core_vulkan.hpp"
#include "rhi_resource_vulkan.hpp"
#include "vulkan_includes.hpp"

namespace mill::rhi
{
    class DeviceVulkan;
    class Pipeline;
    class ImageVulkan;

    class ContextVulkan
    {
    public:
        ContextVulkan(class DeviceVulkan& device);
        ~ContextVulkan();

        void wait_and_begin();
        void end();

        void set_viewport(f32 x, f32 y, f32 w, f32 h, f32 min_depth, f32 max_depth);
        void set_scissor(i32 x, i32 y, u32 w, u32 h);

        void set_pipeline(u64 pipeline_id);

        void set_resource_sets(const std::vector<u64>& resource_set_ids);
        void set_push_constants(u32 offset, u32 size, const void* data);

        void draw(u32 vertex_count);

        void set_index_buffer(HandleBuffer buffer_id, vk::IndexType index_type);
        void set_vertex_buffer(HandleBuffer buffer_id);

        void draw_indexed(u32 index_count);

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

        Shared<Pipeline> m_boundPipeline{};
    };
}
