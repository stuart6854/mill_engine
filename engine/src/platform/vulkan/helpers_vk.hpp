#pragma once

#include "mill/core/base.hpp"
#include "common_headers_vk.hpp"

#include <string_view>

namespace mill::platform::vulkan
{
#pragma region Instance Helpers

    bool is_instance_ext_supported(const std::string_view& ext_name);

    bool is_instance_layer_supported(const std::string_view& layer_name);

#pragma endregion

#pragma region PhysicalDevice Helpers

    auto get_best_dedicated_gpu(vk::Instance instance) -> vk::PhysicalDevice;

#pragma endregion

#pragma region Device Helpers

    bool is_device_ext_supported(vk::PhysicalDevice physical_device, const std::string_view& ext_name);

    auto find_graphics_queue_family(vk::PhysicalDevice physical_device) -> i32;

    auto find_transfer_queue_family(vk::PhysicalDevice physical_device) -> i32;

    void set_object_name(vk::Device device, u64 object, vk::ObjectType type, const char* name);

#define SET_VK_OBJECT_NAME(_device, _typeClass, _object, _name) \
    set_object_name(_device, u64(static_cast<_typeClass>(_object)), _object.objectType, _name);

#define SET_VK_OBJECT_NAME_INDEXED(_device, _typeClass, _object, _name, _index) \
    set_object_name(_device, u64(static_cast<_typeClass>(_object)), _object.objectType, std::format("{} [{}]", _name, _index).c_str());

#pragma endregion

#pragma region Swapchain Helpers

    auto select_swapchain_present_mode(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, bool vsync) -> vk::PresentModeKHR;

    auto select_swapchain_surface_format(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface) -> vk::SurfaceFormatKHR;

    auto select_swapchain_extent(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, u32 width, u32 height) -> vk::Extent2D;

    auto select_swapchain_image_count(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface) -> u32;

#pragma endregion

#pragma region CreateInfo Helpers

    auto get_image_view_create_info_2d(vk::Image image, vk::Format format, u32 base_mip_lvl = 0, u32 mip_lvl_count = 1)
        -> vk::ImageViewCreateInfo;

    auto get_image_subresource_range_2d(u32 base_mip_lvl = 0, u32 mip_lvl_count = 1) -> vk::ImageSubresourceRange;

#pragma endregion

    {
    {

#pragma endregion

}
