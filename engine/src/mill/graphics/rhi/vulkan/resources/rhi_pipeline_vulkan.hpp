#pragma once

#include "mill/graphics/rhi/resources/rhi_pipeline.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    auto to_vulkan(const std::vector<VertexAttribute>& attributes, u32& out_stride) -> std::vector<vk::VertexInputAttributeDescription>;

    auto to_vulkan(PrimitiveTopology topology) -> vk::PrimitiveTopology;
    auto to_vulkan(FillMode fill_mode) -> vk::PolygonMode;
    auto to_vulkan(CullMode cull_mode) -> vk::CullModeFlags;
    auto to_vulkan(FrontFace front_face) -> vk::FrontFace;
}