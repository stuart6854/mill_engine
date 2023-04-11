#include "pipeline_vulkan.hpp"

#include "rhi_resources_vulkan.hpp"
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

        auto convert_vertex_attributes(const std::vector<VertexAttribute>& attributes, u32 binding)
            -> std::vector<vk::VertexInputAttributeDescription>
        {
            std::vector<vk::VertexInputAttributeDescription> out_attributes{};

            u32 offset{ 0 };
            i32 location{ 0 };
            for (auto& attribute : attributes)
            {
                auto& out_attrib = out_attributes.emplace_back();
                out_attrib.setBinding(binding);
                out_attrib.setLocation(location);
                out_attrib.setFormat(convert_format(attribute.format));
                out_attrib.setOffset(offset);

                offset += get_format_size(attribute.format);
                ++location;
            }
            return out_attributes;
        }

        auto get_input_binding(const std::vector<VertexAttribute>& attributes, u32 binding) -> vk::VertexInputBindingDescription
        {
            u32 stride{ 0 };
            for (auto& attribute : attributes)
            {
                stride += get_format_size(attribute.format);
            }

            vk::VertexInputBindingDescription out_binding{};
            out_binding.setBinding(binding);
            out_binding.setInputRate(vk::VertexInputRate::eVertex);
            out_binding.setStride(stride);
            return out_binding;
        }
    }

    PipelineVulkan::PipelineVulkan(InstanceVulkan& device, const PipelineDescription& description) : m_device(device), m_desc(description)
    {
        std::vector<vk::UniqueShaderModule> shader_modules{};
        std::vector<vk::PipelineShaderStageCreateInfo> stage_infos{};
        create_shader_stage(m_device.get_device(), stage_infos, shader_modules, vk::ShaderStageFlagBits::eVertex, m_desc.vs);
        create_shader_stage(m_device.get_device(), stage_infos, shader_modules, vk::ShaderStageFlagBits::eFragment, m_desc.fs);

        vk::PipelineLayoutCreateInfo layout_info{};
        m_layout = m_device.get_device().createPipelineLayoutUnique(layout_info);

        const auto vertex_attributes = convert_vertex_attributes(m_desc.vertexAttributes, 0);
        const auto vertex_binding = get_input_binding(m_desc.vertexAttributes, 0);
        vk::PipelineVertexInputStateCreateInfo vertex_input_state{};
        vertex_input_state.setVertexAttributeDescriptions(vertex_attributes);
        vertex_input_state.setVertexBindingDescriptions(vertex_binding);

        vk::PipelineInputAssemblyStateCreateInfo input_assembly_state{};
        input_assembly_state.setTopology(convert_topology(m_desc.topology));

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

        auto colorAttachmentFormats = convert_formats(m_desc.colorTargets);
        auto depthStencilAttachmentFormat = convert_format(m_desc.depthStencilTarget);
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
