#include "rhi_resource_vulkan.hpp"
#include "mill/graphics/rhi/rhi_resource.hpp"

#include "mill/core/base.hpp"
#include "rhi_core_vulkan.hpp"
#include "vulkan_device.hpp"
#include "vulkan_screen.hpp"
#include "vulkan_view.hpp"
#include "vulkan_helpers.hpp"

namespace mill::rhi
{
    void assign_screen(u64 screen_id, void* window_handle)
    {
        auto& device = get_device();
        ASSERT(device.get_screen(screen_id) == nullptr);

        device.create_screen(screen_id, window_handle);
    }

    void reset_screen(u64 screen_id, u32 width, u32 height, bool vsync)
    {
        const auto& device = get_device();
        auto* screen = device.get_screen(screen_id);
        ASSERT(screen != nullptr);

        screen->reset(width, height, vsync);
    }

    void reset_view(u64 view_id, u32 width, u32 height)
    {
        if (width == 0 || height == 0)
            return;  // Prevent a view with dimensions of (0, 0) - happens when a window is minimized

        auto& device = get_device();
        auto* view = device.get_view(view_id);
        if (view == nullptr)
        {
            device.create_view(view_id);
            view = device.get_view(view_id);
        }
        ASSERT(view != nullptr);

        view->reset(width, height);
    }

    auto to_vulkan(ResourceType type) -> vk::DescriptorType
    {
        switch (type)
        {
            case mill::rhi::ResourceType::eNone: return {};
            case mill::rhi::ResourceType::eUniformBuffer: return vk::DescriptorType::eUniformBuffer;
            case mill::rhi::ResourceType::eTexture: return vk::DescriptorType::eCombinedImageSampler;
            default: ASSERT(("Unknown ResourceType!", false)); break;
        }
        return {};
    }

    auto to_vulkan(ShaderStageFlags stages) -> vk::ShaderStageFlags
    {
        vk::ShaderStageFlags out_stages{};

        if (stages & ShaderStage::eVertex)
            out_stages |= vk::ShaderStageFlagBits::eVertex;

        if (stages & ShaderStage::eFragment)
            out_stages |= vk::ShaderStageFlagBits::eFragment;

        return out_stages;
    }

    auto to_vulkan(Format format) -> vk::Format
    {
        switch (format)
        {
            case mill::rhi::Format::eUndefined: return vk::Format::eUndefined;
            case mill::rhi::Format::eR8: return vk::Format::eR8Unorm;
            case mill::rhi::Format::eR16: return vk::Format::eR16Unorm;
            case mill::rhi::Format::eR32: return vk::Format::eR32Uint;
            case mill::rhi::Format::eRGB32: return vk::Format::eR32G32B32Sfloat;
            case mill::rhi::Format::eRGBA8: return vk::Format::eR8G8B8A8Unorm;
            case mill::rhi::Format::eD16: return vk::Format::eD16Unorm;
            case mill::rhi::Format::eD24S8: return vk::Format::eD24UnormS8Uint;
            case mill::rhi::Format::eD32: return vk::Format::eD32Sfloat;
            case mill::rhi::Format::eD32S8: return vk::Format::eD32SfloatS8Uint;
            default: ASSERT(("Unknown Format!", false)); break;
        }
        return {};
    }

}