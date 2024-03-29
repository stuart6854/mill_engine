#pragma once

#include "mill/core/base.hpp"
#include "pipeline_module.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    class DeviceVulkan;
    class PipelineLayout;

    class Pipeline
    {
    public:
        Pipeline(DeviceVulkan& device);

        void add_module(Shared<PipelineModule> module);
        void set_layout(Shared<PipelineLayout> layout);

        void build();

        /* Getters */

        auto get_pipeline() const -> vk::Pipeline;
        auto get_hash() const -> hasht;

        auto get_layout() const -> const Shared<PipelineLayout>&;

    private:
        auto compute_hash() const -> hasht;

    private:
        DeviceVulkan& m_device;

        vk::UniquePipeline m_pipeline{};
        hasht m_hash{};

        std::vector<Shared<PipelineModule>> m_modules{};
        Shared<PipelineLayout> m_layout{};
    };
}