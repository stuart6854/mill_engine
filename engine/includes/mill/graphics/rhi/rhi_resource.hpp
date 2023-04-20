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
        // RG
        eRG32,
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
}

#include "resources/rhi_pipeline.hpp"
#include "resources/rhi_resource_set.hpp"
#include "resources/rhi_buffer.hpp"
#include "resources/rhi_texture.hpp"
