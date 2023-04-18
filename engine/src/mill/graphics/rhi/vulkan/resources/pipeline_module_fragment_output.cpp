#include "pipeline_module_fragment_output.hpp"

#include "mill/core/base.hpp"
#include "../vulkan_device.hpp"
#include "../vulkan_helpers.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    PipelineModuleFragmentOutput::PipelineModuleFragmentOutput(DeviceVulkan& device) : PipelineModule(device) {}

    void PipelineModuleFragmentOutput::set_color_blend(bool color_blend)
    {
        m_colorBlend = color_blend;
    }

    auto PipelineModuleFragmentOutput::compute_hash() const -> hasht
    {
        hasht hash{};

        hash_combine(hash, m_colorBlend);

        return hash;
    }

    auto PipelineModuleFragmentOutput::build_impl() -> vk::UniquePipeline
    {
        vk::PipelineColorBlendAttachmentState blend_attachment{};
        blend_attachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        blend_attachment.setBlendEnable(m_colorBlend);

        vk::PipelineColorBlendStateCreateInfo color_blend_state{};
        color_blend_state.setAttachments(blend_attachment);

        vk::PipelineMultisampleStateCreateInfo multisample_state{};

        const std::vector<vk::Format> color_formats{ vk::Format::eR8G8B8A8Unorm };
        vk::PipelineRenderingCreateInfo rendering_info{};
        rendering_info.setColorAttachmentFormats(color_formats);
        rendering_info.setDepthAttachmentFormat(vk::Format::eD24UnormS8Uint);

        vk::GraphicsPipelineLibraryCreateInfoEXT library_info{};
        library_info.setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::eFragmentOutputInterface);
        library_info.setPNext(&rendering_info);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.setFlags(vk::PipelineCreateFlagBits::eLibraryKHR | vk::PipelineCreateFlagBits::eRetainLinkTimeOptimizationInfoEXT);
        pipeline_info.setPColorBlendState(&color_blend_state);
        pipeline_info.setPMultisampleState(&multisample_state);
        pipeline_info.setPNext(&library_info);

        return get_device().get_device().createGraphicsPipelineUnique({}, pipeline_info).value;
    }

}
