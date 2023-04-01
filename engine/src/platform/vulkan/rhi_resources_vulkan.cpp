#include "rhi_resources_vulkan.hpp"

#include "mill/platform/rhi.hpp"
#include "rhi_core_vulkan.hpp"
#include "device_vulkan.hpp"
#include "screen_vulkan.hpp"
#include "resource_set_vulkan.hpp"
#include "pipeline_vulkan.hpp"
#include "buffer_vulkan.hpp"

#include <functional>

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

    auto create_resource_set(const ResourceSetDescription& description) -> HandleResourceSet
    {
        ASSERT(g_resources);
        ASSERT(description.bindings.size());

        vk::DescriptorSetLayout layout{};
        u64 desc_hash = std::hash<ResourceSetDescription>{}(description);
        if (g_resources->resourceLayoutMap.contains(desc_hash))
        {
            layout = g_resources->resourceLayoutMap[desc_hash].layout.get();
        }
        else
        {
            std::vector<vk::DescriptorSetLayoutBinding> set_bindings{};
            for (const auto& binding : description.bindings)
            {
                auto& set_binding = set_bindings.emplace_back();
                if (binding.type != ResourceType::eNone && binding.count > 0)
                    set_binding = convert_resource_binding(binding);
            }

            vk::DescriptorSetLayoutCreateInfo layout_info{};
            layout_info.setBindings(set_bindings);
            auto new_layout = get_device().get_device().createDescriptorSetLayoutUnique(layout_info);
            g_resources->resourceLayoutMap[desc_hash] = ResourceLayout{ description, std::move(new_layout) };

            layout = g_resources->resourceLayoutMap[desc_hash].layout.get();
        }

        ASSERT(layout);

        const HandleResourceSet handle = g_resources->nextSetId++;
        g_resources->resourceSetMap[handle] = CreateOwned<ResourceSetVulkan>(get_device(), layout);

        return handle;
    }

    auto create_pipeline(const PipelineDescription& description) -> HandlePipeline
    {
        ASSERT(g_resources);

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

    auto convert_resource_type(ResourceType resource_type) -> vk::DescriptorType
    {
        switch (resource_type)
        {
            case mill::rhi::ResourceType::eNone: return {};
            case mill::rhi::ResourceType::eUniformBuffer: return vk::DescriptorType::eUniformBuffer;
            case mill::rhi::ResourceType::eTexture: return vk::DescriptorType::eCombinedImageSampler;
            default:
                LOG_ERROR("RHI - Vulkan - Unknown ResourceType!");
                ASSERT(false);
                break;
        }
        return {};
    }

    auto convert_shader_stages(ShaderStageFlags shader_stages) -> vk::ShaderStageFlags
    {
        vk::ShaderStageFlags out_stages{};

        if (shader_stages & ShaderStage::eVertex)
            out_stages |= vk::ShaderStageFlagBits::eVertex;

        if (shader_stages & ShaderStage::eFragment)
            out_stages |= vk::ShaderStageFlagBits::eFragment;

        return out_stages;
    }

    auto convert_resource_binding(const ResourceBinding& binding) -> vk::DescriptorSetLayoutBinding
    {
        vk::DescriptorSetLayoutBinding out_binding{};
        out_binding.setDescriptorType(convert_resource_type(binding.type));
        out_binding.setDescriptorCount(binding.count);
        out_binding.setStageFlags(convert_shader_stages(binding.shaderStages));
        return out_binding;
    }

#pragma endregion

}
