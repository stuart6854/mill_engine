#pragma once

#include "mill/core/base.hpp"
#include "mill/graphics/rhi.hpp"
#include "includes_vulkan.hpp"

namespace mill::rhi
{
    class DeviceVulkan;

    class PipelineVulkan
    {
    public:
        PipelineVulkan(DeviceVulkan& device, const PipelineDescription& description);
        ~PipelineVulkan();

        /* Getters */

        auto get_layout() const -> const vk::PipelineLayout&;
        auto get_pipeline() const -> const vk::Pipeline&;

    private:
        DeviceVulkan& m_device;
        PipelineDescription m_desc{};

        vk::UniquePipelineLayout m_layout{};  // #TODO: Cache all layouts (prevent duplicates)
        vk::UniquePipeline m_pipeline{};
    };
}