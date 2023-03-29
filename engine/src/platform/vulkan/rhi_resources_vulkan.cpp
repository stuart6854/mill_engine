#include "rhi_resources_vulkan.hpp"

#include "mill/platform/rhi.hpp"
#include "rhi_core_vulkan.hpp"
#include "screen_vulkan.hpp"

#include <unordered_map>

namespace mill::rhi
{
    Owned<ResourcesVulkan> g_resources{ nullptr };

    void initialise_resources()
    {
        ASSERT(g_resources == nullptr);

        g_resources = CreateOwned<ResourcesVulkan>();
    }

    void shutdown_resources()
    {
        ASSERT(g_resources);

        g_resources.reset();
    }

    auto get_resources() -> ResourcesVulkan&
    {
        return *g_resources;
    }

    void assign_screen(u64 screen, void* window_handle)
    {
        ASSERT(g_resources);
        ASSERT(!g_resources->screenMap.contains(screen));

        g_resources->screenMap[screen] = CreateOwned<ScreenVulkan>(get_device(), window_handle);
    }

    void reset_screen(u64 screen, u32 width, u32 height, bool vsync)
    {
        ASSERT(g_resources);
        ASSERT(g_resources->screenMap.contains(screen));

        auto& screen_inst = g_resources->screenMap.at(screen);
        screen_inst->reset(width, height, vsync);
    }

}
