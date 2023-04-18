#include "rhi_pipeline_vulkan.hpp"

#include "../rhi_core_vulkan.hpp"
#include "../rhi_resource_vulkan.hpp"
#include "pipeline_module_vertex_input.hpp"
#include "pipeline_module_pre_rasterisation.hpp"
#include "pipeline_module_fragment_stage.hpp"
#include "pipeline_module_fragment_output.hpp"
#include "pipeline.hpp"
#include "../vulkan_device.hpp"
#include "../vulkan_helpers.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    auto create_pipeline(const PipelineDescription& description) -> u64
    {
        auto& device = get_device();

        const auto vertex_input_state = to_vulkan(description.vertexInputState);
        auto vertex_input_module = device.get_or_create_pipeline_vertex_input_module(vertex_input_state);

        const auto pre_rasterisation_state = to_vulkan(description.preRasterisationState);
        auto pre_rasterisation_module = device.get_or_create_pipeline_pre_rasterisation_module(pre_rasterisation_state);

        const auto fragment_stage_state = to_vulkan(description.fragmentStageState);
        auto fragment_stage_module = device.get_or_create_pipeline_fragment_stage_module(fragment_stage_state);

        const auto fragment_output_state = to_vulkan(description.fragmentOutputState);
        auto fragment_output_module = device.get_or_create_pipeline_fragment_output_module(fragment_output_state);

        std::vector<Shared<PipelineModule>> modules{
            vertex_input_module,
            pre_rasterisation_module,
            fragment_stage_module,
            fragment_output_module,
        };

        auto pipeline = device.get_or_create_pipeline(modules);

        return pipeline->get_hash();
    }

    auto to_vulkan(const ResourceBinding& binding) -> ResourceBindingVulkan
    {
        ResourceBindingVulkan out_binding{};
        out_binding.type = to_vulkan(binding.type);
        out_binding.count = binding.count;
        out_binding.shaderStages = to_vulkan(binding.shaderStages);
        return out_binding;
    }

    auto to_vulkan(const ResourceSetDescription& input_desc) -> ResourceSetDescriptionVulkan
    {
        ResourceSetDescriptionVulkan out_desc{};

        out_desc.bindings.resize(input_desc.bindings.size());
        for (u32 i = 0; i < out_desc.bindings.size(); ++i)
            out_desc.bindings[i] = to_vulkan(input_desc.bindings[i]);

        return out_desc;
    }

    auto to_vulkan(const std::vector<VertexAttribute>& attributes) -> std::vector<vk::VertexInputAttributeDescription>
    {
        std::vector<vk::VertexInputAttributeDescription> out_attributes{};

        u32 location = 0;
        u32 offset = 0;
        for (auto& attribute : attributes)
        {
            auto& out_attribute = out_attributes.emplace_back();
            out_attribute.setFormat(to_vulkan(attribute.format));
            out_attribute.setBinding(0);
            out_attribute.setLocation(location);
            out_attribute.setOffset(offset);

            ++location;
            offset += vulkan::get_format_byte_size(out_attribute.format);
        }

        return out_attributes;
    }

    auto to_vulkan(PrimitiveTopology topology) -> vk::PrimitiveTopology
    {
        switch (topology)
        {
            case mill::rhi::PrimitiveTopology::ePoints: return vk::PrimitiveTopology::ePointList;
            case mill::rhi::PrimitiveTopology::eLines: return vk::PrimitiveTopology::eLineList;
            case mill::rhi::PrimitiveTopology::eTriangles: return vk::PrimitiveTopology::eTriangleList;
            default: ASSERT(("Unknown PrimitiveTopology!", false)); break;
        }
        return {};
    }

    auto to_vulkan(FillMode fill_mode) -> vk::PolygonMode
    {
        switch (fill_mode)
        {
            case mill::rhi::FillMode::eFill: return vk::PolygonMode::eFill;
            case mill::rhi::FillMode::eWireframe: return vk::PolygonMode::eLine;
            default: ASSERT(("Unknown FillMode!", false)); break;
        }
        return {};
    }

    auto to_vulkan(CullMode cull_mode) -> vk::CullModeFlags
    {
        switch (cull_mode)
        {
            case mill::rhi::CullMode::eNone: return vk::CullModeFlagBits::eNone;
            case mill::rhi::CullMode::eBack: return vk::CullModeFlagBits::eBack;
            case mill::rhi::CullMode::eFront: return vk::CullModeFlagBits::eFront;
            case mill::rhi::CullMode::eFrontAndBack: return vk::CullModeFlagBits::eFrontAndBack;
            default: ASSERT(("Unknown CullMode!", false)); break;
        }
        return {};
    }

    auto to_vulkan(FrontFace front_face) -> vk::FrontFace
    {
        switch (front_face)
        {
            case mill::rhi::FrontFace::eClockwise: return vk::FrontFace::eClockwise;
            case mill::rhi::FrontFace::eCounterClockwise: return vk::FrontFace::eCounterClockwise;
            default: ASSERT(("Unknown FrontFace!", false)); break;
        }
        return {};
    }

    auto to_vulkan(const PipelineVertexInputState& input_state) -> PipelineVertexInputStateVulkan
    {
        PipelineVertexInputStateVulkan out_input_state{};
        out_input_state.attributes = to_vulkan(input_state.attributes);
        out_input_state.topology = to_vulkan(input_state.topology);
        return out_input_state;
    }

    auto to_vulkan(const PipelinePreRasterisationState& input_state) -> PipelinePreRasterisationStateVulkan
    {
        PipelinePreRasterisationStateVulkan out_state{};
        out_state.vertexSpirv = input_state.vertexSpirv;
        out_state.fillMode = to_vulkan(input_state.fillMode);
        out_state.cullMode = to_vulkan(input_state.cullMode);
        out_state.frontFace = to_vulkan(input_state.frontFace);
        out_state.lineWidth = input_state.lineWidth;
        return out_state;
    }

    auto to_vulkan(const PipelineFragmentStageState& input_state) -> PipelineFragmentStageStateVulkan
    {
        PipelineFragmentStageStateVulkan out_state{};
        out_state.fragmentSpirv = input_state.fragmentSpirv;
        out_state.depthTest = input_state.depthTest;
        out_state.stencilTest = input_state.stencilTest;
        return out_state;
    }

    auto to_vulkan(const PipelineFragmentOutputState& input_state) -> PipelineFragmentOutputStateVulkan
    {
        PipelineFragmentOutputStateVulkan out_state{};
        out_state.colorBlend = input_state.enableColorBlend;
        return out_state;
    }

}
