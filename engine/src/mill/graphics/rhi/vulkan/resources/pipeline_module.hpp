#pragma once

#include "mill/core/base.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    class DeviceVulkan;
    class PipelineLayout;

    class PipelineModule
    {
    public:
        PipelineModule(DeviceVulkan& device);

        void set_layout(Shared<PipelineLayout> layout);

        void build();

        /* Getters */

        auto get_device() -> DeviceVulkan&;

        auto get_hash() const -> hasht;
        auto get_library() const -> vk::Pipeline;
        auto get_layout() const -> const Shared<PipelineLayout>&;

    protected:
        virtual auto compute_hash() const -> hasht = 0;

        virtual auto build_impl() -> vk::UniquePipeline = 0;

    private:
        DeviceVulkan& m_device;

        hasht m_hash{};
        vk::UniquePipeline m_library{};

        Shared<PipelineLayout> m_layout{};
    };
}