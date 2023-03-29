#include "screen_vulkan.hpp"

#include "device_vulkan.hpp"
#include "context_vulkan.hpp"
#include "image_vulkan.hpp"
#include "helpers_vulkan.hpp"

namespace mill::rhi
{
    ScreenVulkan::ScreenVulkan(DeviceVulkan& device, void* window_handle) : m_device(device)
    {
#if MILL_WINDOWS
        vk::Win32SurfaceCreateInfoKHR surface_info{};
        surface_info.setHinstance(GetModuleHandleA(nullptr));
        surface_info.setHwnd(static_cast<HWND>(window_handle));
        m_surface = m_device.get_instance().createWin32SurfaceKHRUnique(surface_info);
#endif

        for (auto& semaphore : m_acquireSemaphores)
        {
            semaphore = m_device.get_device().createSemaphoreUnique({});
        }
    }

    void ScreenVulkan::reset(u32 width, u32 height, bool vsync)
    {
        ASSERT(m_device.get_device());
        ASSERT(m_surface);

        // Handle minimized windows
        m_isMinimized = width == 0 || height == 0;
        if (m_isMinimized)
            return;

        m_device.wait_idle();

        vk::UniqueSwapchainKHR old_swapchain = std::move(m_swapchain);

        m_swapchain.reset();
        m_backbuffers.clear();

        m_presentMode = vulkan::select_swapchain_present_mode(m_device.get_physical_device(), m_surface.get(), vsync);
        m_surfaceFormat = vulkan::select_swapchain_surface_format(m_device.get_physical_device(), m_surface.get());
        m_extent = vulkan::select_swapchain_extent(m_device.get_physical_device(), m_surface.get(), width, height);
        auto minImageCount = vulkan::select_swapchain_image_count(m_device.get_physical_device(), m_surface.get());

        vk::SwapchainCreateInfoKHR swapchain_info{};
        swapchain_info.setSurface(m_surface.get());
        swapchain_info.setMinImageCount(minImageCount);
        swapchain_info.setImageFormat(m_surfaceFormat.format);
        swapchain_info.setImageColorSpace(m_surfaceFormat.colorSpace);
        swapchain_info.setImageExtent(m_extent);
        swapchain_info.setImageArrayLayers(1);
        swapchain_info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst);
        swapchain_info.setPresentMode(m_presentMode);
        swapchain_info.setOldSwapchain(old_swapchain.get());
        m_swapchain = m_device.get_device().createSwapchainKHRUnique(swapchain_info);

        auto images = m_device.get_device().getSwapchainImagesKHR(m_swapchain.get());
        for (auto& image : images)
        {
            auto& backbuffer = m_backbuffers.emplace_back();
            backbuffer =
                CreateOwned<ImageVulkan>(m_device, image, swapchain_info.imageUsage, vk::Extent3D(m_extent, 1), m_surfaceFormat.format);
        }
    }

    void ScreenVulkan::acquire_image()
    {
        // Handle minimized screens
        if (is_minimized())
            return;

        m_semaphoreIndex = (m_semaphoreIndex + 1) % CAST_U32(m_acquireSemaphores.size());

        auto result =
            m_device.get_device().acquireNextImageKHR(m_swapchain.get(), u64_max, m_acquireSemaphores[m_semaphoreIndex].get(), {});
        ASSERT(result.result == vk::Result::eSuccess);  // #TODO: Temporary. Handle resizing swapchain
        m_imageIndex = result.value;

        get_backbuffer().set_layout(vk::ImageLayout::eUndefined);
    }

    auto ScreenVulkan::get_backbuffer() -> ImageVulkan&
    {
        return *m_backbuffers[m_imageIndex];
    }

    auto ScreenVulkan::get_swap_chain() const -> vk::SwapchainKHR
    {
        return m_swapchain.get();
    }

    auto ScreenVulkan::get_image_index() const -> u32
    {
        return m_imageIndex;
    }

    auto ScreenVulkan::get_acquire_semaphore() const -> const vk::Semaphore&
    {
        return m_acquireSemaphores[m_semaphoreIndex].get();
    }

    bool ScreenVulkan::is_minimized() const
    {
        return m_isMinimized;
    }

}
