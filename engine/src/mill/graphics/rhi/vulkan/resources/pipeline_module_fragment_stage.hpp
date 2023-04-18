#pragma once

#include "mill/core/base.hpp"
#include "pipeline_module.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    class DeviceVulkan;
    class PipelineLayout;

    class PipelineModuleFragmentStage : public PipelineModule
    {
    public:
        PipelineModuleFragmentStage(DeviceVulkan& device);

        void set_fragment_spirv(const std::vector<u32>& spirv);
        void set_depth_test(bool depth_test);
        void set_stencil_test(bool stencil_test);

    protected:
        auto compute_hash() const -> hasht override;

        auto build_impl() -> vk::UniquePipeline override;

    private:
        std::vector<u32> m_fragmentSpirv{};
        bool m_depthTest{};
        bool m_stencilTest{};
    };
}
