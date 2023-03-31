#pragma once

#include "mill/core/base.hpp"
#include "mill/platform/rhi.hpp"
#include "includes_vulkan.hpp"
#include "rhi_core_vulkan.hpp"

#include <array>

namespace mill::rhi
{
    class ImageVulkan;

    class ContextVulkan
    {
    public:
        ContextVulkan(class DeviceVulkan& device);
        ~ContextVulkan() = default;

        void wait_and_begin();
        void end();

        void set_viewport(f32 x, f32 y, f32 w, f32 h, f32 min_depth, f32 max_depth);
        void set_scissor(i32 x, i32 y, u32 w, u32 h);

        void set_pipeline(HandlePipeline pipeline);

        void draw(u32 vertex_count);

        void associate_screen(u64 screen);

        void transition_image(const vk::ImageMemoryBarrier2& barrier);

        void blit(ImageVulkan& srcImage, ImageVulkan& dstImage);

        /* Getters */

        auto get_cmd() -> vk::CommandBuffer&;
        auto get_completed_semaphore() -> vk::Semaphore&;
        auto get_fence() -> vk::Fence&;
        bool get_was_recorded();

        auto get_associated_screens() const -> const std::vector<u64>&;

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

        std::vector<u64> m_associatedScreens{};
        class PipelineVulkan* m_boundPipeline{ nullptr };
    };
}
