#include "rhi_pipeline_vulkan.hpp"

#include "../rhi_resource_vulkan.hpp"
#include "../vulkan_helpers.hpp"

namespace mill::rhi
{
    auto to_vulkan(const std::vector<VertexAttribute>& attributes, u32& out_stride) -> std::vector<vk::VertexInputAttributeDescription>
    {
        std::vector<vk::VertexInputAttributeDescription> out_attributes{};

        u32 location = 0;
        out_stride = 0;
        for (auto& attribute : attributes)
        {
            auto& out_attribute = out_attributes.emplace_back();
            out_attribute.setFormat(to_vulkan(attribute.format));
            out_attribute.setBinding(0);
            out_attribute.setLocation(location);
            out_attribute.setOffset(out_stride);

            ++location;
            out_stride += vulkan::get_format_byte_size(out_attribute.format);
        }

        return out_attributes;
    }

    auto to_vulkan(PrimitiveTopology topology) -> vk::PrimitiveTopology
    {
        switch (topology)
        {
            case mill::rhi::PrimitiveTopology::ePoints: return vk::PrimitiveTopology::ePointList;
            case mill::rhi::PrimitiveTopology::eLines: return vk::PrimitiveTopology::eLineList;
            case mill::rhi::PrimitiveTopology::eTriangles: return vk::PrimitiveTopology::eTriangleList;
            default: ASSERT(("Unknown PrimitiveTopology!", false)); break;
        }
        return {};
    }

    auto to_vulkan(FillMode fill_mode) -> vk::PolygonMode
    {
        switch (fill_mode)
        {
            case mill::rhi::FillMode::eFill: return vk::PolygonMode::eFill;
            case mill::rhi::FillMode::eWireframe: return vk::PolygonMode::eLine;
            default: ASSERT(("Unknown FillMode!", false)); break;
        }
        return {};
    }

    auto to_vulkan(CullMode cull_mode) -> vk::CullModeFlags
    {
        switch (cull_mode)
        {
            case mill::rhi::CullMode::eNone: return vk::CullModeFlagBits::eNone;
            case mill::rhi::CullMode::eBack: return vk::CullModeFlagBits::eBack;
            case mill::rhi::CullMode::eFront: return vk::CullModeFlagBits::eFront;
            case mill::rhi::CullMode::eFrontAndBack: return vk::CullModeFlagBits::eFrontAndBack;
            default: ASSERT(("Unknown CullMode!", false)); break;
        }
        return {};
    }

    auto to_vulkan(FrontFace front_face) -> vk::FrontFace
    {
        switch (front_face)
        {
            case mill::rhi::FrontFace::eClockwise: return vk::FrontFace::eClockwise;
            case mill::rhi::FrontFace::eCounterClockwise: return vk::FrontFace::eCounterClockwise;
            default: ASSERT(("Unknown FrontFace!", false)); break;
        }
        return {};
    }

}
