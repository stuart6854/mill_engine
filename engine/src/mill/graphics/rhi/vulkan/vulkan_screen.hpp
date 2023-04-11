#pragma once

#include "mill/core/base.hpp"
#include "rhi_core_vulkan.hpp"
#include "vulkan_includes.hpp"
#include "vulkan_image.hpp"

namespace mill::rhi
{
    class ScreenVulkan
    {
    public:
        ScreenVulkan(class DeviceVulkan& device, u64 id, void* window_handle);
        ~ScreenVulkan() = default;

        void reset(u32 width, u32 height, bool vsync);

        void acquire_image();

        /* Getters */

        auto get_id() const -> u64;

        auto get_swap_chain() const -> vk::SwapchainKHR;
        auto get_image_index() const -> u32;
        auto get_acquire_semaphore() const -> const vk::Semaphore&;

        auto get_backbuffer() -> ImageVulkan&;

        bool is_minimized() const;

    private:
        DeviceVulkan& m_device;
        const u64 m_id{};

        vk::UniqueSurfaceKHR m_surface{};
        vk::UniqueSwapchainKHR m_swapchain{};

        vk::PresentModeKHR m_presentMode{};
        vk::SurfaceFormatKHR m_surfaceFormat{};
        vk::Extent2D m_extent{};
        std::vector<Owned<ImageVulkan>> m_backbuffers{};
        u32 m_imageIndex{};

        std::array<vk::UniqueSemaphore, g_FrameBufferCount> m_acquireSemaphores{};
        u32 m_semaphoreIndex{};

        bool m_isMinimized{};
    };
}