#pragma once

#include "mill/core/base.hpp"
#include "pipeline_module.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    class DeviceVulkan;
    class PipelineLayout;

    class PipelineModulePreRasterisation : public PipelineModule
    {
    public:
        PipelineModulePreRasterisation(DeviceVulkan& device);

        void set_vertex_spirv(const std::vector<u32>& spirv);
        void set_polygon_mode(vk::PolygonMode polygon_mode);
        void set_cull_mode(vk::CullModeFlags cull_mode);
        void set_front_face(vk::FrontFace front_face);
        void set_line_width(f32 line_width);

    protected:
        auto compute_hash() const -> hasht override;

        auto build_impl() -> vk::UniquePipeline override;

    private:
        std::vector<u32> m_vertexSpirv{};
        vk::PolygonMode m_polygonMode{};
        vk::CullModeFlags m_cullMode{};
        vk::FrontFace m_frontFace{};
        f32 m_lineWidth{ 1.0f };
    };
}
