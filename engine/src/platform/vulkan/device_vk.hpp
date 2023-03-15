#pragma once

#include "mill/core/base.hpp"
#include "common_headers_vk.hpp"
#include "upload_context_vk.hpp"
#include "resources_vk.hpp"

namespace mill::platform::vulkan
{
    struct AppInfo
    {
        std::string appName{};
    };

    struct Receipt
    {
        vk::Semaphore semaphore{};
        vk::Fence fence{};
    };

    class ContextVulkan;

    class DeviceVulkan
    {
    public:
        DeviceVulkan();
        ~DeviceVulkan();

        DISABLE_COPY_AND_MOVE(DeviceVulkan);

        bool initialise(const AppInfo& app_info);
        bool shutdown();

        void begin_frame();
        void end_frame();
        void present(Receipt* wait_for = nullptr);

        void wait_for(Receipt& receipt);
        void wait_idle();

        auto submit_context(ContextVulkan& context, Receipt* wait_for = nullptr) -> Receipt;

        bool add_surface(void* surface_handle, i32 width, i32 height);
        void remove_surface(void* surface_handle);

        auto create_context() -> Owned<ContextVulkan>;
        auto create_pipeline(const PipelineInit& pipeline_init) -> Owned<PipelineVulkan>;
        auto create_buffer(const BufferInit& buffer_init) -> BufferVulkan*;

        void destroy_context(Owned<ContextVulkan> context);
        void destroy_pipeline(Owned<PipelineVulkan> pipeline);
        void destroy_buffer(BufferVulkan* buffer);

        /* Getters */

        auto get_device() const -> vk::Device;

        auto get_graphics_queue_family() const -> i32;
        auto get_graphics_queue() const -> vk::Queue;
        auto get_transfer_queue_family() const -> i32;
        auto get_transfer_queue() const -> vk::Queue;

        auto get_upload_context() const -> UploadContextVulkan&;

        auto get_current_back_buffer(void* surface_handle) -> ImageVulkan*;

    private:
        bool setup_per_frame_resource();

        struct Surface;
        auto get_surface_from_handle(void* handle) -> Surface*;

        auto create_surface(void* surface_handle) -> vk::SurfaceKHR;

        bool create_swapchain(Surface& surface, vk::SwapchainKHR old_swapchain);
        void destroy_swapchain(Surface& surface);

        void create_buffer(BufferVulkan& buffer);
        void destroy_buffer(BufferVulkan& buffer);

    private:
        vk::DynamicLoader m_loader{};
        vk::Instance m_instance{};
        vk::DebugUtilsMessengerEXT m_debugMessenger{};
        vk::PhysicalDevice m_physicalDevice{};
        vk::Device m_device{};
        vma::Allocator m_allocator{};

        i32 m_graphicsQueueFamily{ -1 };
        vk::Queue m_graphicsQueue{};

        i32 m_transferQueueFamily{ -1 };
        vk::Queue m_transferQueue{};

        Owned<UploadContextVulkan> m_uploadContext{ nullptr };

        // std::array<vk::Fence, g_FrameBufferCount> m_endOfFrameFences{};
        std::array<Receipt, g_FrameBufferCount> m_endOfFrameRecipts{};
        u32 m_frameIndex{};

        struct Surface
        {
            sizet index{};
            void* handle{ nullptr };
            vk::SurfaceKHR surface{};
            i32 width{};
            i32 height{};
            vk::SwapchainKHR swapchain{};
            std::vector<Owned<ImageVulkan>> backBufferImages{};
            u32 imageIndex{};

            vk::Semaphore imageReadySemaphore{};
        };
        std::vector<Owned<Surface>> m_surfaces{};

        std::unordered_map<u64, BufferVulkan> m_buffers{};
        u64 m_nextBufferId{ 1 };
    };
}