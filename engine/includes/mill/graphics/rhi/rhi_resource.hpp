#pragma once

#include "mill/core/base.hpp"
#include "mill/utility/flags.hpp"

#include <vector>

namespace mill::rhi
{
    enum class Format : u16
    {
        eUndefined,
        // Grey scale
        eR8,
        eR16,
        eR32,
        // RGB
        eRGB32,
        // RGBA
        eRGBA8,
        // Depth/Stencil
        eD16,
        eD24S8,
        eD32,
        eD32S8,
    };

    void assign_screen(u64 screen_id, void* window_handle);

    /* Update a screens back-buffer size. */
    void reset_screen(u64 screen_id, u32 width, u32 height, bool vsync);

    void reset_view(u64 view_id, u32 width, u32 height);

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
    };
}

#include "resources/rhi_pipeline.hpp"
#include "resources/rhi_buffer.hpp"
