#pragma once

#include "mill/core/base.hpp"
#include "../rhi_resource.hpp"

#include <glm/ext/vector_uint3.hpp>

namespace mill::rhi
{
    struct TextureDescription
    {
        glm::uvec3 dimensions{};
        Format format{};
        u32 mipLevels{ 1 };
    };

    auto create_texture(const TextureDescription& description) -> u64;
    void write_texture(u64 texture_id, u32 mip_level, const void* data);
    void generate_mip_maps(u64 texture_id);
}