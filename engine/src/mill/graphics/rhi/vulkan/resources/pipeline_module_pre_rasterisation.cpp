#include "pipeline_module_pre_rasterisation.hpp"

#include "mill/core/base.hpp"
#include "pipeline_layout.hpp"
#include "../vulkan_device.hpp"
#include "../vulkan_helpers.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    PipelineModulePreRasterisation::PipelineModulePreRasterisation(DeviceVulkan& device) : PipelineModule(device) {}

    void PipelineModulePreRasterisation::set_vertex_spirv(const std::vector<u32>& spirv)
    {
        m_vertexSpirv = spirv;
    }

    void PipelineModulePreRasterisation::set_polygon_mode(vk::PolygonMode polygon_mode)
    {
        m_polygonMode = polygon_mode;
    }

    void PipelineModulePreRasterisation::set_cull_mode(vk::CullModeFlags cull_mode)
    {
        m_cullMode = cull_mode;
    }

    void PipelineModulePreRasterisation::set_front_face(vk::FrontFace front_face)
    {
        m_frontFace = front_face;
    }

    void PipelineModulePreRasterisation::set_line_width(f32 line_width)
    {
        m_lineWidth = line_width;
    }

    auto PipelineModulePreRasterisation::compute_hash() const -> hasht
    {
        hasht hash{};

        hash_combine(hash, m_vertexSpirv.size());
        for (auto& x : m_vertexSpirv)
            hash_combine(hash, x);

        hash_combine(hash, m_polygonMode);
        hash_combine(hash, m_cullMode);
        hash_combine(hash, m_frontFace);
        hash_combine(hash, m_lineWidth);

        return hash;
    }

    auto PipelineModulePreRasterisation::build_impl() -> vk::UniquePipeline
    {
        vk::ShaderModuleCreateInfo module_info{};
        module_info.setCode(m_vertexSpirv);

        vk::PipelineShaderStageCreateInfo stage_info{};
        stage_info.setPNext(&module_info);
        stage_info.setStage(vk::ShaderStageFlagBits::eVertex);
        stage_info.setPName("main");

        const std::vector<vk::DynamicState> dynamic_states{
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor,
        };
        vk::PipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.setDynamicStates(dynamic_states);

        vk::PipelineViewportStateCreateInfo viewport_state{};
        viewport_state.setViewportCount(1);
        viewport_state.setScissorCount(1);

        vk::PipelineRasterizationStateCreateInfo rasterisation_state{};
        rasterisation_state.setPolygonMode(m_polygonMode);
        rasterisation_state.setCullMode(m_cullMode);
        rasterisation_state.setFrontFace(m_frontFace);
        rasterisation_state.setLineWidth(m_lineWidth);

        vk::GraphicsPipelineLibraryCreateInfoEXT library_info{};
        library_info.setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::ePreRasterizationShaders);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.setFlags(vk::PipelineCreateFlagBits::eLibraryKHR | vk::PipelineCreateFlagBits::eRetainLinkTimeOptimizationInfoEXT);
        pipeline_info.setLayout(get_layout()->get_layout());
        pipeline_info.setStages(stage_info);
        pipeline_info.setPDynamicState(&dynamic_state);
        pipeline_info.setPViewportState(&viewport_state);
        pipeline_info.setPRasterizationState(&rasterisation_state);
        pipeline_info.setPNext(&library_info);

        return get_device().get_device().createGraphicsPipelineUnique({}, pipeline_info).value;
    }

}
