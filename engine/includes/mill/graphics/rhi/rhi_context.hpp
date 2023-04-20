#pragma once

#include "mill/core/base.hpp"
#include "rhi_resource.hpp"

#include <glm/ext/vector_float4.hpp>

namespace mill::rhi
{
    void begin_context(u64 context_id);
    void end_context(u64 context_id);

    void begin_view(u64 context_id, u64 view_id, const glm::vec4& clear_color = { 1, 1, 1, 1 }, f32 clear_depth = 1.0f);
    void end_view(u64 context_id, u64 view_id);

    void set_viewport(u64 context_id, f32 x, f32 y, f32 w, f32 h, f32 min_depth = 0.0f, f32 max_depth = 1.0f);
    void set_scissor(u64 context_id, i32 x, i32 y, u32 w, u32 h);

    void set_pipeline(u64 context_id, u64 pipeline_id);

    enum class IndexType
    {
        eU16,
        eU32,
    };
    void set_index_buffer(u64 context_id, HandleBuffer buffer_id, IndexType index_type);
    void set_vertex_buffer(u64 context_id, HandleBuffer buffer_id);

    void set_resource_sets(u64 context_id, const std::vector<u64>& resource_set_ids);
    void set_push_constants(u64 context_id, u32 offset, u32 size, const void* data);

    void draw(u64 context_id, u32 vertex_count);
    void draw_indexed(u64 context_id, u32 index_count, u32 instance_count, u32 index_offset, u32 vertex_offset);

    void blit_to_screen(u64 context_id, u64 screen, u64 view);
}