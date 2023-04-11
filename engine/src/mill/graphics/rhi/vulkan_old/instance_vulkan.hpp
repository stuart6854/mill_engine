#pragma once

#include "mill/core/base.hpp"
#include "includes_vulkan.hpp"

namespace mill::rhi
{
    class InstanceVulkan
    {
    public:
        InstanceVulkan();
        ~InstanceVulkan();

        bool initialise();
        void shutdown();

        void wait_idle() const;

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
    };
}