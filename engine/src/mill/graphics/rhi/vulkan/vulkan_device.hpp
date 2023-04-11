#pragma once

#include "mill/core/base.hpp"
#include "vulkan_includes.hpp"

#include <vector>
#include <unordered_map>

namespace mill::rhi
{
    class ScreenVulkan;
    class ContextVulkan;
    class ViewVulkan;

    class DeviceVulkan
    {
    public:
        DeviceVulkan();
        ~DeviceVulkan();

        bool initialise();
        void shutdown();
        void wait_idle() const;

        /* Resources */

        void create_screen(u64 screen_id, void* window_handle);
        void destroy_screen(u64 screen_id);
        auto get_screen(u64 screen_id) const -> ScreenVulkan*;
        auto get_all_screens() const -> const std::vector<ScreenVulkan*>&;

        void create_context(u64 context_id);
        void destroy_context(u64 context_id);
        auto get_context(u64 context_id) const -> ContextVulkan*;
        auto get_all_contexts() const -> const std::vector<ContextVulkan*>&;

        void create_view(u64 view_id);
        void destroy_view(u64 view_id);
        auto get_view(u64 view_id) const -> ViewVulkan*;

        /* Getters */

        auto get_instance() -> vk::Instance&;
        auto get_physical_device() -> vk::PhysicalDevice&;
        auto get_device() -> vk::Device&;

        auto get_graphics_queue_family() -> i32;
        auto get_compute_queue_family() -> i32;
        auto get_transfer_queue_family() -> i32;

        auto get_graphics_queue() -> vk::Queue&;
        auto get_compute_queue() -> vk::Queue&;
        auto get_transfer_queue() -> vk::Queue&;

        auto get_allocator() -> vma::Allocator&;
        auto get_descriptor_pool() -> vk::DescriptorPool&;

    private:
        bool init_instance();
        bool init_physical_device();
        bool init_device();

    private:
        vk::DynamicLoader m_loader{};
        vk::UniqueInstance m_instance;
        vk::UniqueDebugUtilsMessengerEXT m_debugMessenger{};
        vk::PhysicalDevice m_physicalDevice;
        vk::UniqueDevice m_device{};

        i32 m_graphicsQueueFamily{};
        vk::Queue m_graphicsQueue{};
        i32 m_computeQueueFamily{};
        vk::Queue m_computeQueue{};
        i32 m_transferQueueFamily{};
        vk::Queue m_transferQueue{};

        vma::UniqueAllocator m_allocator{};
        vk::UniqueDescriptorPool m_descriptorPool{};

        std::unordered_map<u64, Owned<ScreenVulkan>> m_screens{};
        std::vector<ScreenVulkan*> m_allScreens{};

        std::unordered_map<u64, Owned<ContextVulkan>> m_contexts{};
        std::vector<ContextVulkan*> m_allContexts{};

        std::unordered_map<u64, Owned<ViewVulkan>> m_views{};
    };
}