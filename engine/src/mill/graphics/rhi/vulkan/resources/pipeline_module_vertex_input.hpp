#pragma once

#include "mill/core/base.hpp"
#include "pipeline_module.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    class DeviceVulkan;
    class PipelineLayout;

    class PipelineModuleVertexInput : public PipelineModule
    {
    public:
        PipelineModuleVertexInput(DeviceVulkan& device);

        void set_input_attributes(const std::vector<vk::VertexInputAttributeDescription>& attributes);
        void set_topology(vk::PrimitiveTopology topology);

    protected:
        auto compute_hash() const -> hasht override;

        auto build_impl() -> vk::UniquePipeline override;

    private:
        std::vector<vk::VertexInputAttributeDescription> m_attributes{};
        vk::PrimitiveTopology m_topology{};
    };
}
