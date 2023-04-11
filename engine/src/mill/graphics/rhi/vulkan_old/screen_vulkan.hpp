#pragma once

#include "mill/core/base.hpp"
#include "includes_vulkan.hpp"
#include "rhi_core_vulkan.hpp"
#include "image_vulkan.hpp"

namespace mill::rhi
{
    class ContextVulkan;

    class ScreenVulkan
    {
    public:
        ScreenVulkan(class InstanceVulkan& device, void* window_handle);
        ~ScreenVulkan() = default;

        void reset(u32 width, u32 height, bool vsync);

        void acquire_image();

        /* Getters */

        auto get_swap_chain() const -> vk::SwapchainKHR;
        auto get_image_index() const -> u32;
        auto get_acquire_semaphore() const -> const vk::Semaphore&;

        auto get_backbuffer() -> ImageVulkan&;

        bool is_minimized() const;

    private:
        InstanceVulkan& m_device;
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