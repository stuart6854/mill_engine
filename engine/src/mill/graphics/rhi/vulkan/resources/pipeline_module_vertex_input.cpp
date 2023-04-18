#include "pipeline_module_vertex_input.hpp"

#include "mill/core/base.hpp"
#include "../vulkan_device.hpp"
#include "../vulkan_helpers.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    PipelineModuleVertexInput::PipelineModuleVertexInput(DeviceVulkan& device) : PipelineModule(device) {}

    void PipelineModuleVertexInput::set_input_attributes(const std::vector<vk::VertexInputAttributeDescription>& attributes)
    {
        m_attributes = attributes;
    }

    void PipelineModuleVertexInput::set_topology(vk::PrimitiveTopology topology)
    {
        m_topology = topology;
    }

    auto PipelineModuleVertexInput::compute_hash() const -> hasht
    {
        hasht hash{};

        hash_combine(hash, m_attributes.size());
        for (const auto& attribute : m_attributes)
        {
            hash_combine(hash, attribute.binding);
            hash_combine(hash, attribute.location);
            hash_combine(hash, attribute.offset);
            hash_combine(hash, attribute.format);
        }

        hash_combine(hash, m_topology);

        return hash;
    }

    auto PipelineModuleVertexInput::build_impl() -> vk::UniquePipeline
    {
        u32 stride = 0;
        for (const auto& attribute : m_attributes)
            stride += vulkan::get_format_byte_size(attribute.format);

        vk::VertexInputBindingDescription binding{};
        binding.setBinding(0);
        binding.setInputRate(vk::VertexInputRate::eVertex);
        binding.setStride(stride);

        vk::PipelineVertexInputStateCreateInfo vertex_input_state{};
        vertex_input_state.setVertexAttributeDescriptions(m_attributes);
        if (!m_attributes.empty())
            vertex_input_state.setVertexBindingDescriptions(binding);

        vk::PipelineInputAssemblyStateCreateInfo input_assembly_state{};
        input_assembly_state.setTopology(m_topology);

        vk::GraphicsPipelineLibraryCreateInfoEXT library_info{};
        library_info.setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::eVertexInputInterface);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.setFlags(vk::PipelineCreateFlagBits::eLibraryKHR | vk::PipelineCreateFlagBits::eRetainLinkTimeOptimizationInfoEXT);
        pipeline_info.setPVertexInputState(&vertex_input_state);
        pipeline_info.setPInputAssemblyState(&input_assembly_state);
        pipeline_info.setPNext(&library_info);

        return get_device().get_device().createGraphicsPipelineUnique({}, pipeline_info).value;
    }
}
