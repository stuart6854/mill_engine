#include "pipeline.hpp"

#include "mill/core/base.hpp"
#include "../vulkan_device.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    Pipeline::Pipeline(DeviceVulkan& device) : m_device(device) {}

    void Pipeline::add_module(Shared<PipelineModule> module)
    {
        m_modules.push_back(std::move(module));
    }

    void Pipeline::build()
    {
        // #TODO: Merge pipeline layouts

        std::vector<vk::Pipeline> libraries(m_modules.size());
        for (u32 i = 0; i < m_modules.size(); ++i)
            libraries[i] = m_modules[i]->get_library();

        vk::PipelineLibraryCreateInfoKHR linking_info{};
        linking_info.setLibraries(libraries);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        // pipeline_info.setLayout(layout);
        pipeline_info.setPNext(&linking_info);
        // if (g_PipelineLinkTimeOptimisation)
        pipeline_info.setFlags(vk::PipelineCreateFlagBits::eLinkTimeOptimizationEXT);

        m_pipeline = m_device.get_device().createGraphicsPipelineUnique({}, pipeline_info).value;

        m_hash = compute_hash();
    }

    auto Pipeline::get_pipeline() const -> vk::Pipeline
    {
        return m_pipeline.get();
    }

    auto Pipeline::get_hash() const -> hasht
    {
        if (m_hash)
            return m_hash;

        return compute_hash();
    }

    auto Pipeline::compute_hash() const -> hasht
    {
        hasht hash{};

        for (const auto& module : m_modules)
            hash_combine(hash, module->get_hash());

        return hash;
    }

}
