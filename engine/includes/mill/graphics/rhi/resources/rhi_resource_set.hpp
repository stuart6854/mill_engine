#pragma once

#include "mill/core/base.hpp"
#include "../rhi_resource.hpp"

#include <vector>

namespace mill::rhi
{
    enum class ShaderStage : u8
    {
        eNone = 0,
        eVertex = 1u << 0,
        eFragment = 1u << 1,
    };
    using ShaderStageFlags = Flags<ShaderStage>;

    enum class ResourceType : u8
    {
        eNone,
        eUniformBuffer,
        eTexture,
    };

    struct ResourceBinding
    {
        ResourceType type{};
        u32 count{ 1 };
        ShaderStageFlags shaderStages{};
    };

    struct ResourceSetDescription
    {
        std::vector<ResourceBinding> bindings{};
        bool buffer{};
    };

    auto create_resource_set(const ResourceSetDescription& description) -> u64;

    void bind_buffer_to_resource_set(u64 resource_set_id, u32 binding, u64 buffer_id);
}