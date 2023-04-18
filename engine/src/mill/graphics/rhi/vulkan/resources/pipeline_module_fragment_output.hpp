#pragma once

#include "mill/core/base.hpp"
#include "pipeline_module.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    class DeviceVulkan;
    class PipelineLayout;

    class PipelineModuleFragmentOutput : public PipelineModule
    {
    public:
        PipelineModuleFragmentOutput(DeviceVulkan& device);

        void set_color_blend(bool color_blend);

    protected:
        auto compute_hash() const -> hasht override;

        auto build_impl() -> vk::UniquePipeline override;

    private:
        bool m_colorBlend{};
    };
}
