#pragma once

#include "mill/core/base.hpp"
#include "rhi_resource.hpp"

#include <glm/ext/vector_float4.hpp>

namespace mill::rhi
{
    void begin_contex(u64 context);
    void end_context(u64 context);

    void begin_view(u64 context, u64 view, const glm::vec4& clear_color = { 1, 1, 1, 1 }, f32 clear_depth = 1.0f);
    void end_view(u64 context, u64 view);

    void set_viewport(u64 context, f32 x, f32 y, f32 w, f32 h, f32 min_depth = 0.0f, f32 max_depth = 1.0f);
    void set_scissor(u64 context, i32 x, i32 y, u32 w, u32 h);

    void set_pipeline(u64 context, HandlePipeline pipeline);

    enum class IndexType
    {
        eU16,
        eU32,
    };
    void set_index_buffer(u64 context, HandleBuffer buffer, IndexType index_type);
    void set_vertex_buffer(u64 context, HandleBuffer buffer);

    void draw(u64 context, u32 vertex_count);
    void draw_indexed(u64 context, u32 index_count);

    void blit_to_screen(u64 context, u64 screen, u64 view);
}