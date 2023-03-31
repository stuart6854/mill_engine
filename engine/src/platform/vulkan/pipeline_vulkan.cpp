#include "pipeline_vulkan.hpp"

#include "device_vulkan.hpp"

namespace mill::rhi
{
    namespace
    {
        void create_shader_module(vk::Device& device, std::vector<vk::UniqueShaderModule>& out, const std::vector<u32>& spirv)
        {
            vk::ShaderModuleCreateInfo module_info{};
            module_info.setCode(spirv);
            out.push_back(device.createShaderModuleUnique(module_info));
        }

        void create_shader_stage(vk::Device& device,
                                 std::vector<vk::PipelineShaderStageCreateInfo>& stage_out,
                                 std::vector<vk::UniqueShaderModule>& module_out,
                                 vk::ShaderStageFlagBits stage,
                                 const std::vector<u32>& spirv)
        {
            if (spirv.empty())
                return;

            create_shader_module(device, module_out, spirv);

            auto& stage_info = stage_out.emplace_back();
            stage_info.setModule(module_out.back().get());
            stage_info.setPName("main");
            stage_info.setStage(stage);
        }

        auto to_vulkan(PrimitiveTopology topology) -> vk::PrimitiveTopology
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

        auto to_vulkan(ImageFormat format) -> vk::Format
        {
            switch (format)
            {
                case mill::rhi::ImageFormat::eUndefined: return vk::Format::eUndefined;
                case mill::rhi::ImageFormat::eR8: return vk::Format::eR8Unorm;
                case mill::rhi::ImageFormat::eR16: return vk::Format::eR16Unorm;
                case mill::rhi::ImageFormat::eR32: return vk::Format::eR32Uint;
                case mill::rhi::ImageFormat::eRGBA8: return vk::Format::eR8G8B8A8Unorm;
                case mill::rhi::ImageFormat::eD16: return vk::Format::eD16Unorm;
                case mill::rhi::ImageFormat::eD24S8: return vk::Format::eD24UnormS8Uint;
                case mill::rhi::ImageFormat::eD32: return vk::Format::eD32Sfloat;
                case mill::rhi::ImageFormat::eD32S8: return vk::Format::eD32SfloatS8Uint;
                default:
                    LOG_ERROR("RHI - Vulkan - Unknown ImageFormat!");
                    ASSERT(false);
                    break;
            }
            return {};
        }

        auto to_vulkan(const std::vector<ImageFormat>& formats) -> std::vector<vk::Format>
        {
            std::vector<vk::Format> out_formats{};
            out_formats.reserve(formats.size());
            for (auto& format : formats)
            {
                out_formats.push_back(to_vulkan(format));
            }
            return out_formats;
        }
    }

    PipelineVulkan::PipelineVulkan(DeviceVulkan& device, const PipelineDescription& description) : m_device(device), m_desc(description)
    {
        std::vector<vk::UniqueShaderModule> shader_modules{};
        std::vector<vk::PipelineShaderStageCreateInfo> stage_infos{};
        create_shader_stage(m_device.get_device(), stage_infos, shader_modules, vk::ShaderStageFlagBits::eVertex, m_desc.vs);
        create_shader_stage(m_device.get_device(), stage_infos, shader_modules, vk::ShaderStageFlagBits::eFragment, m_desc.fs);

        vk::PipelineLayoutCreateInfo layout_info{};
        m_layout = m_device.get_device().createPipelineLayoutUnique(layout_info);

        vk::PipelineVertexInputStateCreateInfo vertex_input_state{};

        vk::PipelineInputAssemblyStateCreateInfo input_assembly_state{};
        input_assembly_state.setTopology(to_vulkan(m_desc.topology));

        vk::PipelineTessellationStateCreateInfo tessellation_state{};

        vk::PipelineViewportStateCreateInfo viewport_state{};
        viewport_state.setViewportCount(1);
        viewport_state.setScissorCount(1);

        vk::PipelineRasterizationStateCreateInfo rasterization_state{};
        rasterization_state.setLineWidth(m_desc.rasterizerDesc.lineWidth);

        vk::PipelineMultisampleStateCreateInfo multisample_state{};

        vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{};
        depth_stencil_state.setDepthTestEnable(m_desc.depthStencilDesc.depthEnabled);
        depth_stencil_state.setStencilTestEnable(m_desc.depthStencilDesc.stencilEnabled);

        vk::PipelineColorBlendAttachmentState attachment_state{};
        attachment_state.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        attachment_state.setBlendEnable(false);
        vk::PipelineColorBlendStateCreateInfo color_blend_state{};
        color_blend_state.setAttachments(attachment_state);

        const std::vector<vk::DynamicState> DynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.setDynamicStates(DynamicStates);

        auto colorAttachmentFormats = to_vulkan(m_desc.colorTargets);
        auto depthStencilAttachmentFormat = to_vulkan(m_desc.depthStencilTarget);
        vk::PipelineRenderingCreateInfo rendering_info{};
        rendering_info.setColorAttachmentFormats(colorAttachmentFormats);
        rendering_info.setDepthAttachmentFormat(depthStencilAttachmentFormat);
        rendering_info.setStencilAttachmentFormat(depthStencilAttachmentFormat);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.setStages(stage_infos);
        pipeline_info.setPVertexInputState(&vertex_input_state);
        pipeline_info.setPInputAssemblyState(&input_assembly_state);
        pipeline_info.setPTessellationState(&tessellation_state);
        pipeline_info.setPViewportState(&viewport_state);
        pipeline_info.setPRasterizationState(&rasterization_state);
        pipeline_info.setPMultisampleState(&multisample_state);
        pipeline_info.setPDepthStencilState(&depth_stencil_state);
        pipeline_info.setPColorBlendState(&color_blend_state);
        pipeline_info.setPDynamicState(&dynamic_state);
        pipeline_info.setLayout(m_layout.get());
        pipeline_info.setPNext(&rendering_info);

        m_pipeline = m_device.get_device().createGraphicsPipelineUnique({}, pipeline_info).value;
    }

    PipelineVulkan::~PipelineVulkan() = default;

    auto PipelineVulkan::get_layout() const -> const vk::PipelineLayout&
    {
        return m_layout.get();
    }

    auto PipelineVulkan::get_pipeline() const -> const vk::Pipeline&
    {
        return m_pipeline.get();
    }
}
