#include "pipeline_module.hpp"

#include "mill/core/base.hpp"
#include "../vulkan_device.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    PipelineModule::PipelineModule(DeviceVulkan& device) : m_device(device) {}

    void PipelineModule::set_layout(Shared<PipelineLayout> layout)
    {
        m_layout = layout;
    }

    void PipelineModule::build()
    {
        m_library = build_impl();
        m_hash = compute_hash();
    }

    auto PipelineModule::get_device() -> DeviceVulkan&
    {
        return m_device;
    }

    auto PipelineModule::get_hash() const -> hasht
    {
        if (m_hash)
            return m_hash;

        return compute_hash();
    }

    auto PipelineModule::get_library() const -> vk::Pipeline
    {
        return m_library.get();
    }

    auto PipelineModule::get_layout() const -> const Shared<PipelineLayout>&
    {
        return m_layout;
    }
}
