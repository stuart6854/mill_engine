#include "rhi_resources_vulkan.hpp"

#include "mill/platform/rhi.hpp"
#include "rhi_core_vulkan.hpp"
#include "screen_vulkan.hpp"
#include "pipeline_vulkan.hpp"
#include "buffer_vulkan.hpp"

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

    auto create_pipeline(const PipelineDescription& description) -> HandlePipeline
    {
        const HandlePipeline handle = g_resources->nextPipelineId++;

        auto& pipeline = g_resources->pipelineMap[handle];
        pipeline = CreateOwned<PipelineVulkan>(get_device(), description);

        return handle;
    }

    auto create_buffer(const BufferDescription& description) -> HandleBuffer
    {
        ASSERT(g_resources);
        ASSERT(description.size);

        const HandleBuffer handle = g_resources->nextBufferId++;

        auto& buffer = g_resources->bufferMap[handle];
        buffer = CreateOwned<BufferVulkan>(get_device(), description);

        return handle;
    }

}
