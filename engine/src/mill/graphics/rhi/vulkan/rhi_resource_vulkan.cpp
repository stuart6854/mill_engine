#include "mill/graphics/rhi/rhi_resource.hpp"

#include "rhi_core_vulkan.hpp"
#include "vulkan_device.hpp"
#include "vulkan_screen.hpp"
#include "vulkan_view.hpp"

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
}