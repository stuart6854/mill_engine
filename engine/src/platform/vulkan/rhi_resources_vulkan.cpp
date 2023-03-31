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

    void write_buffer(HandleBuffer buffer, u64 offset, u64 size, const void* data)
    {
        ASSERT(g_resources);
        ASSERT(buffer);
        ASSERT(g_resources->bufferMap.contains(buffer));

        auto& buffer_inst = g_resources->bufferMap[buffer];
        ASSERT(buffer_inst != nullptr);

        const bool isHostVisible =
            buffer_inst->get_mem_usage() == MemoryUsage::eHost || buffer_inst->get_mem_usage() == MemoryUsage::eDeviceHostVisble;
        if (!isHostVisible)
        {
            // #TODO: Transfer with staging buffer
            ASSERT(false);
        }
        else
        {
            buffer_inst->write(offset, size, data);
        }
    }

#pragma region Utility

    auto convert_topology(PrimitiveTopology topology) -> vk::PrimitiveTopology
    {
        switch (topology)
        {
            case mill::rhi::PrimitiveTopology::ePoints: return vk::PrimitiveTopology::ePointList;
            case mill::rhi::PrimitiveTopology::eLines: return vk::PrimitiveTopology::eLineList;
            case mill::rhi::PrimitiveTopology::eTriangles: return vk::PrimitiveTopology::eTriangleList;
            default:
                LOG_ERROR("RHI - Vulkan - Unknown PrimitiveTopology!");
                ASSERT(false);
                break;
        }
        return {};
    }

    auto convert_format(Format format) -> vk::Format
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
            default:
                LOG_ERROR("RHI - Vulkan - Unknown Format!");
                ASSERT(false);
                break;
        }
        return {};
    }

    auto convert_formats(const std::vector<Format>& formats) -> std::vector<vk::Format>
    {
        std::vector<vk::Format> out_formats{};
        out_formats.reserve(formats.size());
        for (auto& format : formats)
        {
            out_formats.push_back(convert_format(format));
        }
        return out_formats;
    }

    auto get_format_size(Format format) -> u32
    {
        switch (format)
        {
            case mill::rhi::Format::eUndefined: return 0;
            case mill::rhi::Format::eR8: return 1;
            case mill::rhi::Format::eR16: return 2;
            case mill::rhi::Format::eR32: return 4;
            case mill::rhi::Format::eRGB32: return 4 * 3;
            case mill::rhi::Format::eRGBA8: return 1 * 4;
            default:
                LOG_ERROR("RHI - Vulkan - Cannot get size of unknown Format!");
                ASSERT(false);
                break;
        }
        return 0;
    }

#pragma endregion

}
