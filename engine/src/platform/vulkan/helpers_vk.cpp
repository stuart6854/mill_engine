#include "helpers_vk.hpp"

namespace mill::platform::vulkan
{
    bool is_instance_ext_supported(const std::string_view& ext_name)
    {
        auto inst_exts = vk::enumerateInstanceExtensionProperties();
        for (const auto& ext : inst_exts)
        {
            if (ext.extensionName == ext_name)
            {
                return true;
            }
        }
        return false;
    }

    bool is_instance_layer_supported(const std::string_view& layer_name)
    {
        auto inst_layers = vk::enumerateInstanceLayerProperties();
        for (const auto& layer : inst_layers)
        {
            if (layer.layerName == layer_name)
            {
                return true;
            }
        }
        return false;
    }

    auto get_best_dedicated_gpu(vk::Instance instance) -> vk::PhysicalDevice
    {
        auto physicalDevices = instance.enumeratePhysicalDevices();

        u64 best_score{};
        vk::PhysicalDevice best_gpu{};
        for (auto gpu : physicalDevices)
        {
            u64 score = 0;

            auto props = gpu.getProperties();
            if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            {
                score += 1000;
            }
            else if (props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
            {
                score += 500;
            }

            score += props.limits.maxImageDimension2D;

            auto mem_props = gpu.getMemoryProperties();
            for (auto& heap : mem_props.memoryHeaps)
            {
                if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
                {
                    score += heap.size;
                    break;
                }
            }

            if (score > best_score)
            {
                best_score = score;
                best_gpu = gpu;
            }
        }
        return best_gpu;
    }

    bool is_device_ext_supported(vk::PhysicalDevice physical_device, const std::string_view& ext_name)
    {
        auto device_exts = physical_device.enumerateDeviceExtensionProperties();
        for (const auto& ext : device_exts)
        {
            if (ext.extensionName == ext_name)
            {
                return true;
            }
        }
        return false;
    }

    auto find_graphics_queue_family(vk::PhysicalDevice physical_device) -> i32
    {
        auto queue_props = physical_device.getQueueFamilyProperties();

        // Try find a Graphics queue without compute
        for (u32 i = 0; i < queue_props.size(); ++i)
        {
            const auto& family = queue_props[i];
            if (family.queueFlags & vk::QueueFlagBits::eGraphics && !(family.queueFlags & vk::QueueFlagBits::eCompute))
            {
                return i;
            }
        }

        // Now, just return the first Graphics queue available
        for (u32 i = 0; i < queue_props.size(); ++i)
        {
            const auto& family = queue_props[i];
            if (family.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                return i;
            }
        }

        return -1;
    }

    auto find_transfer_queue_family(vk::PhysicalDevice physical_device) -> i32
    {
        auto queue_props = physical_device.getQueueFamilyProperties();

        // Try find a Transfer queue without compute & graphics
        for (u32 i = 0; i < queue_props.size(); ++i)
        {
            const auto& family = queue_props[i];
            if (family.queueFlags & vk::QueueFlagBits::eTransfer && !(family.queueFlags & vk::QueueFlagBits::eCompute) &&
                !(family.queueFlags & vk::QueueFlagBits::eGraphics))
            {
                return i;
            }
        }

        // Now, just return the first Graphics queue available
        for (u32 i = 0; i < queue_props.size(); ++i)
        {
            const auto& family = queue_props[i];
            if (family.queueFlags & vk::QueueFlagBits::eTransfer)
            {
                return i;
            }
        }

        return -1;
    }

    void set_object_name(vk::Device device, u64 object, vk::ObjectType type, const char* name)
    {
#ifndef MILL_DISTRO
        vk::DebugUtilsObjectNameInfoEXT name_info{};
        name_info.setObjectHandle(object);
        name_info.setObjectType(type);
        name_info.setPObjectName(name);
        device.setDebugUtilsObjectNameEXT(name_info);
#endif
    }

    auto select_swapchain_present_mode(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, bool vsync) -> vk::PresentModeKHR
    {
        auto present_modes = physical_device.getSurfacePresentModesKHR(surface);

        // If vsync is not requested, return Immediate, if supported.
        if (!vsync)
        {
            for (auto mode : present_modes)
            {
                if (mode == vk::PresentModeKHR::eImmediate)
                {
                    return vk::PresentModeKHR::eImmediate;
                }
            }
        }

        // Try to return eMailbox, if supported.
        for (auto mode : present_modes)
        {
            if (mode == vk::PresentModeKHR::eMailbox)
            {
                return vk::PresentModeKHR::eMailbox;
            }
        }

        // FIFO is required to be supported, according to Vulkan specs.
        return vk::PresentModeKHR::eFifo;
    }

    auto select_swapchain_surface_format(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface) -> vk::SurfaceFormatKHR
    {
        auto supported_formats = physical_device.getSurfaceFormatsKHR(surface);
        ASSERT(!supported_formats.empty());

        std::vector preferred_formats{
            vk::Format::eB8G8R8A8Srgb,
            vk::Format::eR8G8B8A8Srgb,
            vk::Format::eB8G8R8A8Unorm,
            vk::Format::eR8G8B8A8Unorm,
        };

        for (auto preferred_format : preferred_formats)
        {
            for (auto supported_format : supported_formats)
            {
                if (supported_format.format == preferred_format)
                {
                    return supported_format;
                }
            }
        }

        return { vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear };
    }

    auto select_swapchain_extent(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, u32 width, u32 height) -> vk::Extent2D
    {
        auto caps = physical_device.getSurfaceCapabilitiesKHR(surface);

        width = std::clamp(width, caps.minImageExtent.width, caps.maxImageExtent.width);
        height = std::clamp(height, caps.minImageExtent.height, caps.maxImageExtent.height);

        return vk::Extent2D(width, height);
    }

    auto select_swapchain_image_count(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface) -> u32
    {
        auto caps = physical_device.getSurfaceCapabilitiesKHR(surface);

        u32 image_count = caps.minImageCount + 1;
        if (caps.maxImageCount > 0 && image_count > caps.maxImageCount)
        {
            image_count = caps.maxImageCount;
        }
        return image_count;
    }

    auto get_image_view_create_info_2d(vk::Image image, vk::Format format, u32 base_mip_lvl, u32 mip_lvl_count) -> vk::ImageViewCreateInfo
    {
        vk::ImageViewCreateInfo view_info{};
        view_info.setImage(image);
        view_info.setFormat(format);
        view_info.setViewType(vk::ImageViewType::e2D);
        view_info.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
        view_info.subresourceRange.setBaseMipLevel(base_mip_lvl);
        view_info.subresourceRange.setLevelCount(mip_lvl_count);
        view_info.subresourceRange.setBaseArrayLayer(0);
        view_info.subresourceRange.setLayerCount(1);
        return view_info;
    }

    auto get_image_subresource_range_2d(u32 base_mip_lvl, u32 mip_lvl_count) -> vk::ImageSubresourceRange
    {
        vk::ImageSubresourceRange range{};
        range.setAspectMask(vk::ImageAspectFlagBits::eColor);
        range.setBaseMipLevel(base_mip_lvl);
        range.setLevelCount(mip_lvl_count);
        range.setBaseArrayLayer(0);
        range.setLayerCount(1);
        return range;
    }
}
