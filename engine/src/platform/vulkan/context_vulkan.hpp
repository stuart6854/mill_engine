#pragma once

#include "mill/core/base.hpp"
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
    };
}
