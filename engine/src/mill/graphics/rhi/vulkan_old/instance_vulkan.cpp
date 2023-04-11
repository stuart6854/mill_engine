#include "instance_vulkan.hpp"

#include "mill/core/base.hpp"
#include "helpers_vulkan.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace mill::rhi
{
    constexpr auto g_VulkanAPIVersion = VK_API_VERSION_1_3;

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_message_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                          VkDebugUtilsMessageTypeFlagsEXT /*message_type*/,
                                                          const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                          void* /*user_data*/)
    {
        if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            LOG_WARN("Vulkan - MessageId = {} : [{}]\n{}\n",
                     callback_data->messageIdNumber,
                     callback_data->pMessageIdName,
                     callback_data->pMessage);
        }
        else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            LOG_ERROR("Vulkan - MessageId = {} : [{}]\n{}\n",
                      callback_data->messageIdNumber,
                      callback_data->pMessageIdName,
                      callback_data->pMessage);
        }
        return VK_FALSE;
    }

    InstanceVulkan::InstanceVulkan() = default;

    InstanceVulkan::~InstanceVulkan()
    {
        ASSERT(!m_instance || !m_device);
    }

    bool InstanceVulkan::initialise()
    {
        ASSERT(!m_instance && !m_device);

        if (!init_instance())
            return false;

        if (!init_physical_device())
            return false;

        if (!init_device())
            return false;

        // Allocator
        {
            vma::AllocatorCreateInfo alloc_info{};
            alloc_info.setInstance(*m_instance);
            alloc_info.setPhysicalDevice(m_physicalDevice);
            alloc_info.setDevice(*m_device);
            alloc_info.setVulkanApiVersion(g_VulkanAPIVersion);
            m_allocator = vma::createAllocatorUnique(alloc_info);
        }

        // Descriptor Pool
        {
            std::vector<vk::DescriptorPoolSize> pool_sizes{
                { vk::DescriptorType::eUniformBuffer, 100 },
                { vk::DescriptorType::eStorageBuffer, 100 },
                { vk::DescriptorType::eCombinedImageSampler, 100 },
            };

            vk::DescriptorPoolCreateInfo pool_info{};
            pool_info.setMaxSets(100);
            pool_info.setPoolSizes(pool_sizes);
            pool_info.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
            m_descriptorPool = m_device->createDescriptorPoolUnique(pool_info);
        }

        return true;
    }

    void InstanceVulkan::shutdown()
    {
        m_descriptorPool.reset();
        m_allocator.reset();
        m_device.reset();
        m_debugMessenger.reset();
        m_instance.reset();
    }

    void InstanceVulkan::wait_idle() const
    {
        m_device->waitIdle();
    }

    auto InstanceVulkan::get_instance() -> vk::Instance&
    {
        return *m_instance;
    }

    auto InstanceVulkan::get_physical_device() -> vk::PhysicalDevice&
    {
        return m_physicalDevice;
    }

    auto InstanceVulkan::get_device() -> vk::Device&
    {
        return *m_device;
    }

    auto InstanceVulkan::get_graphics_queue_family() -> i32
    {
        return m_graphicsQueueFamily;
    }

    auto InstanceVulkan::get_compute_queue_family() -> i32
    {
        return m_computeQueueFamily;
    }

    auto InstanceVulkan::get_transfer_queue_family() -> i32
    {
        return m_transferQueueFamily;
    }

    auto InstanceVulkan::get_graphics_queue() -> vk::Queue&
    {
        return m_graphicsQueue;
    }

    auto InstanceVulkan::get_compute_queue() -> vk::Queue&
    {
        return m_computeQueue;
    }

    auto InstanceVulkan::get_transfer_queue() -> vk::Queue&
    {
        return m_transferQueue;
    }

    auto InstanceVulkan::get_allocator() -> vma::Allocator&
    {
        return m_allocator.get();
    }

    auto InstanceVulkan::get_descriptor_pool() -> vk::DescriptorPool&
    {
        return m_descriptorPool.get();
    }

    bool InstanceVulkan::init_instance()
    {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

        vk::ApplicationInfo vk_app_info{};
        vk_app_info.setPEngineName("Mill_Engine");
        vk_app_info.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
        vk_app_info.setPApplicationName("mill_engine_app");
        vk_app_info.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
        vk_app_info.setApiVersion(g_VulkanAPIVersion);

        std::vector instance_exts
        {
#if MILL_DEBUG
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif

                VK_KHR_SURFACE_EXTENSION_NAME,

#if MILL_WINDOWS
                VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
        };

        for (const auto& ext : instance_exts)
        {
            if (!vulkan::is_instance_ext_supported(ext))
            {
                LOG_ERROR("DeviceVulkan - The instance extension '{}' is not supported!", ext);
            }
        }

        std::vector instance_layers
        {
#if MILL_DEBUG
            "VK_LAYER_KHRONOS_validation",
#endif

                "VK_LAYER_KHRONOS_synchronization2",
        };

        for (const auto& layer : instance_layers)
        {
            if (!vulkan::is_instance_layer_supported(layer))
            {
                LOG_ERROR("DeviceVulkan - The instance layer '{}' is not supported!", layer);
            }
        }

        vk::InstanceCreateInfo instance_info{};
        instance_info.setPApplicationInfo(&vk_app_info);
        instance_info.setPEnabledExtensionNames(instance_exts);
        instance_info.setPEnabledLayerNames(instance_layers);

#if MILL_DEBUG
        vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_info{};
        debug_messenger_info.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning);
        debug_messenger_info.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
        debug_messenger_info.setPfnUserCallback(&debug_message_callback);
        instance_info.setPNext(&debug_messenger_info);
#endif
        m_instance = vk::createInstanceUnique(instance_info);
        if (!m_instance)
        {
            LOG_ERROR("DeviceVulkan - Failed to create instance!");
            return false;
        }

        VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_instance);

#if MILL_DEBUG
        m_debugMessenger = m_instance->createDebugUtilsMessengerEXTUnique(debug_messenger_info);
#endif

        LOG_DEBUG("DeviceVulkan - The Vulkan instance has been created.");
        LOG_DEBUG("DeviceVulkan - The following extensions were enabled:");
        for (const auto& ext : instance_exts)
        {
            LOG_DEBUG("\t{}", ext);
        }
        LOG_DEBUG("DeviceVulkan - The following layers were enabled:");
        for (const auto& layer : instance_layers)
        {
            LOG_DEBUG("\t{}", layer);
        }

        return true;
    }

    bool InstanceVulkan::init_physical_device()
    {
        m_physicalDevice = vulkan::get_best_dedicated_gpu(*m_instance);
        if (!m_physicalDevice)
        {
            LOG_ERROR("DeviceVulkan - Failed to select a physical device!");
            return false;
        }

        auto gpu_props = m_physicalDevice.getProperties();
        LOG_INFO(
            "{}, Driver {}.{}", gpu_props.deviceName, VK_VERSION_MAJOR(gpu_props.driverVersion), VK_VERSION_MINOR(gpu_props.driverVersion));

        return true;
    }

    bool InstanceVulkan::init_device()
    {
        std::vector<const char*> device_exts{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        for (const auto& ext : device_exts)
        {
            if (!vulkan::is_device_ext_supported(m_physicalDevice, ext))
            {
                LOG_ERROR("DeviceVulkan - The device extension '{}' is not supported!", ext);
            }
        }

        vk::PhysicalDeviceFeatures features{};

        vk::PhysicalDeviceSynchronization2Features sync2_features{};
        sync2_features.setSynchronization2(true);

        vk::PhysicalDeviceDynamicRenderingFeatures dyn_rendering_features{};
        dyn_rendering_features.setDynamicRendering(true);
        dyn_rendering_features.setPNext(&sync2_features);

        // #TODO: Check if descriptor indexing is supported
        vk::PhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features{};
        descriptor_indexing_features.setDescriptorBindingPartiallyBound(true);
        descriptor_indexing_features.setRuntimeDescriptorArray(true);
        descriptor_indexing_features.setPNext(&dyn_rendering_features);

        m_graphicsQueueFamily = vulkan::find_graphics_queue_family(m_physicalDevice);
        m_transferQueueFamily = vulkan::find_transfer_queue_family(m_physicalDevice);

        std::vector<vk::DeviceQueueCreateInfo> queue_infos{};
        const f32 queue_priority = 1.0f;
        if (m_graphicsQueueFamily != -1)
        {
            auto& queue_info = queue_infos.emplace_back();
            queue_info.setQueueFamilyIndex(m_graphicsQueueFamily);
            queue_info.setQueueCount(1);
            queue_info.setQueuePriorities(queue_priority);
        }
        if (m_transferQueueFamily != -1)
        {
            auto& queue_info = queue_infos.emplace_back();
            queue_info.setQueueFamilyIndex(m_transferQueueFamily);
            queue_info.setQueueCount(1);
            queue_info.setQueuePriorities(queue_priority);
        }

        vk::DeviceCreateInfo device_info{};
        device_info.setPEnabledExtensionNames(device_exts);
        device_info.setPEnabledFeatures(&features);
        device_info.setQueueCreateInfos(queue_infos);
        device_info.setPNext(&descriptor_indexing_features);

        m_device = m_physicalDevice.createDeviceUnique(device_info);
        if (!m_device)
        {
            LOG_ERROR("DeviceVulkan - Failed to create device!");
            return false;
        }

        LOG_DEBUG("DeviceVulkan - The Vulkan device has been created.");
        if (!device_exts.empty())
        {
            LOG_DEBUG("DeviceVulkan - The following extensions were enabled:");
            for (const auto& ext : device_exts)
            {
                LOG_DEBUG("\t{}", ext);
            }
        }

        VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_device);

        if (m_graphicsQueueFamily != -1)
        {
            m_graphicsQueue = m_device->getQueue(m_graphicsQueueFamily, 0);
            SET_VK_OBJECT_NAME(*m_device, VkQueue, m_graphicsQueue, "Main Queue (Graphics)");
        }
        if (m_transferQueueFamily != -1)
        {
            m_transferQueue = m_device->getQueue(m_transferQueueFamily, 0);
            SET_VK_OBJECT_NAME(*m_device, VkQueue, m_transferQueue, "Main Queue (Transfer)");
        }

        return true;
    }

}
