#include "vulkan_image.hpp"

#include "vulkan_includes.hpp"
#include "vulkan_device.hpp"
#include "vulkan_helpers.hpp"

namespace mill::rhi
{
    ImageVulkan::ImageVulkan(DeviceVulkan& device, vk::Image image, vk::ImageUsageFlags usage, vk::Extent3D dimensions, vk::Format format)
        : m_device(device), m_image(image), m_usage(usage), m_dimensions(dimensions), m_format(format), m_mipLevels(1)
    {
        vk::ImageViewCreateInfo view_info = vulkan::get_image_view_create_info_2d(m_image, m_format, 0, m_mipLevels);
        m_view = m_device.get_device().createImageView(view_info);
    }

    ImageVulkan::ImageVulkan(DeviceVulkan& device, vk::ImageUsageFlags usage, vk::Extent3D dimensions, vk::Format format, u32 mip_levels)
        : m_device(device), m_usage(usage), m_dimensions(dimensions), m_format(format), m_mipLevels(mip_levels)
    {
        ASSERT(m_dimensions.width > 0 && m_dimensions.height > 0 && m_dimensions.depth > 0);
        ASSERT(mip_levels > 0);

        vk::ImageCreateInfo image_info{};
        image_info.setUsage(m_usage);
        image_info.setExtent(m_dimensions);
        image_info.setFormat(m_format);
        image_info.setMipLevels(m_mipLevels);
        image_info.setArrayLayers(1);
        image_info.setImageType(vk::ImageType::e2D);

        vma::AllocationCreateInfo alloc_info{};
        alloc_info.setUsage(vma::MemoryUsage::eAuto);

        auto [image, allocation] = m_device.get_allocator().createImage(image_info, alloc_info);
        m_image = image;
        m_allocation = allocation;

        vk::ImageViewCreateInfo view_info = vulkan::get_image_view_create_info_2d(m_image, m_format, 0, m_mipLevels);
        m_view = m_device.get_device().createImageView(view_info);
    }

    ImageVulkan::~ImageVulkan()
    {
        m_device.get_device().destroy(m_view);
        if (m_image && m_allocation)
        {
            m_device.get_allocator().destroyImage(m_image, m_allocation);
        }
    }

    void ImageVulkan::set_layout(vk::ImageLayout layout)
    {
        m_layout = layout;
    }

    auto ImageVulkan::get_image() -> vk::Image&
    {
        return m_image;
    }

    auto ImageVulkan::get_view() -> vk::ImageView&
    {
        return m_view;
    }

    auto ImageVulkan::get_usage() -> vk::ImageUsageFlags
    {
        return m_usage;
    }

    auto ImageVulkan::get_dimensions() -> vk::Extent3D
    {
        return m_dimensions;
    }

    auto ImageVulkan::get_format() -> vk::Format
    {
        return m_format;
    }

    auto ImageVulkan::get_mip_levels() -> u32
    {
        return m_mipLevels;
    }

    auto ImageVulkan::get_layout() -> vk::ImageLayout
    {
        return m_layout;
    }

}
