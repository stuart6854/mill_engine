#pragma once

#include "mill/core/base.hpp"
#include "mill/graphics/rhi.hpp"
#include "includes_vulkan.hpp"

namespace mill::rhi
{
#if 0
    struct ResourceLayout
    {
        ResourceSetDescription desc{};
        vk::UniqueDescriptorSetLayout layout{};
    };
#endif

    struct ResourcesVulkan
    {
        std::unordered_map<u64, Owned<class ScreenVulkan>> screenMap{};

        // std::unordered_map<u64, ResourceLayout> resourceLayoutMap{};
        std::unordered_map<u64, Owned<class ResourceSetVulkan>> resourceSetMap{};
        u64 nextSetId{ 1 };

        std::unordered_map<u64, Owned<class PipelineVulkan>> pipelineMap{};
        u64 nextPipelineId{ 1 };

        std::unordered_map<u64, Owned<class BufferVulkan>> bufferMap{};
        u64 nextBufferId{ 1 };
    };

    void initialise_resources();
    void shutdown_resources();

    auto get_resources() -> ResourcesVulkan&;

#pragma region Utility

    auto convert_topology(PrimitiveTopology topology) -> vk::PrimitiveTopology;
    auto convert_format(Format format) -> vk::Format;
    auto convert_formats(const std::vector<Format>& formats) -> std::vector<vk::Format>;

    auto get_format_size(Format format) -> u32;

#if 0
	auto convert_resource_type(ResourceType resource_type) -> vk::DescriptorType;
    auto convert_shader_stages(ShaderStageFlags shader_stages) -> vk::ShaderStageFlags;
    auto convert_resource_binding(const ResourceBinding& binding) -> vk::DescriptorSetLayoutBinding;
#endif  // 0

#pragma endregion

}

#if 0
namespace std
{
    template <>
    struct hash<mill::rhi::ResourceSetDescription>
    {
        std::size_t operator()(const mill::rhi::ResourceSetDescription& desc) const
        {
            using std::hash;
            using std::size_t;

            size_t value{};

            mill::hash_combine(value, desc.bindings.size());
            for (const auto& binding : desc.bindings)
            {
                mill::hash_combine(value, static_cast<uint8_t>(binding.type));
                mill::hash_combine(value, binding.count);
                mill::hash_combine(value, static_cast<uint8_t>(binding.shaderStages));
            }

            return value;
        }
    };
}
#endif  // 0
