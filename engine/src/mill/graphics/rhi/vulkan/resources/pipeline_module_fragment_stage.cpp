#include "pipeline_module_fragment_stage.hpp"

#include "mill/core/base.hpp"
#include "pipeline_layout.hpp"
#include "../vulkan_device.hpp"
#include "../vulkan_helpers.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    PipelineModuleFragmentStage::PipelineModuleFragmentStage(DeviceVulkan& device) : PipelineModule(device) {}

    void PipelineModuleFragmentStage::set_fragment_spirv(const std::vector<u32>& spirv)
    {
        m_fragmentSpirv = spirv;
    }

    void PipelineModuleFragmentStage::set_depth_test(bool depth_test)
    {
        m_depthTest = depth_test;
    }

    void PipelineModuleFragmentStage::set_stencil_test(bool stencil_test)
    {
        m_stencilTest = stencil_test;
    }

    auto PipelineModuleFragmentStage::compute_hash() const -> hasht
    {
        hasht hash{};

        hash_combine(hash, m_fragmentSpirv.size());
        for (auto& x : m_fragmentSpirv)
            hash_combine(hash, x);

        hash_combine(hash, m_depthTest);
        hash_combine(hash, m_stencilTest);

        return hash;
    }

    auto PipelineModuleFragmentStage::build_impl() -> vk::UniquePipeline
    {
        vk::ShaderModuleCreateInfo module_info{};
        module_info.setCode(m_fragmentSpirv);

        vk::PipelineShaderStageCreateInfo stage_info{};
        stage_info.setPNext(&module_info);
        stage_info.setStage(vk::ShaderStageFlagBits::eFragment);
        stage_info.setPName("main");

        vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{};
        depth_stencil_state.setDepthTestEnable(m_depthTest);
        depth_stencil_state.setStencilTestEnable(m_stencilTest);

        vk::PipelineMultisampleStateCreateInfo multisample_state{};

        vk::GraphicsPipelineLibraryCreateInfoEXT library_info{};
        library_info.setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::eFragmentShader);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.setFlags(vk::PipelineCreateFlagBits::eLibraryKHR | vk::PipelineCreateFlagBits::eRetainLinkTimeOptimizationInfoEXT);
        pipeline_info.setLayout(get_layout()->get_layout());
        pipeline_info.setStages(stage_info);
        pipeline_info.setPDepthStencilState(&depth_stencil_state);
        pipeline_info.setPMultisampleState(&multisample_state);
        pipeline_info.setPNext(&library_info);

        return get_device().get_device().createGraphicsPipelineUnique({}, pipeline_info).value;
    }

}
