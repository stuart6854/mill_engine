#pragma once

#include "mill/core/base.hpp"
#include "mill/graphics/rhi/resources/rhi_pipeline.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    class DeviceVulkan;

    struct ResourceBindingVulkan
    {
        vk::DescriptorType type{};
        u32 count{};
        vk::ShaderStageFlags shaderStages{};
    };
    auto to_vulkan(const ResourceBinding& binding) -> ResourceBindingVulkan;

    struct ResourceSetDescriptionVulkan
    {
        std::vector<ResourceBindingVulkan> bindings{};
    };
    auto to_vulkan(const ResourceSetDescription& input_desc) -> ResourceSetDescriptionVulkan;

    auto to_vulkan(const std::vector<VertexAttribute>& attributes) -> std::vector<vk::VertexInputAttributeDescription>;
    auto to_vulkan(PrimitiveTopology topology) -> vk::PrimitiveTopology;
    auto to_vulkan(FillMode fill_mode) -> vk::PolygonMode;
    auto to_vulkan(CullMode cull_mode) -> vk::CullModeFlags;
    auto to_vulkan(FrontFace front_face) -> vk::FrontFace;

    struct PipelineVertexInputStateVulkan
    {
        std::vector<vk::VertexInputAttributeDescription> attributes{};
        vk::PrimitiveTopology topology{};
    };
    auto to_vulkan(const PipelineVertexInputState& input_state) -> PipelineVertexInputStateVulkan;

    struct PipelinePreRasterisationStateVulkan
    {
        std::vector<u32> vertexSpirv{};
        vk::PolygonMode fillMode{};
        vk::CullModeFlags cullMode{};
        vk::FrontFace frontFace{};
        f32 lineWidth = 1.0f;
    };
    auto to_vulkan(const PipelinePreRasterisationState& input_state) -> PipelinePreRasterisationStateVulkan;

    struct PipelineFragmentStageStateVulkan
    {
        std::vector<u32> fragmentSpirv{};
        bool depthTest{};
        bool stencilTest{};
    };
    auto to_vulkan(const PipelineFragmentStageState& input_state) -> PipelineFragmentStageStateVulkan;

    struct PipelineFragmentOutputStateVulkan
    {
        bool colorBlend{};
    };
    auto to_vulkan(const PipelineFragmentOutputState& input_state) -> PipelineFragmentOutputStateVulkan;

}