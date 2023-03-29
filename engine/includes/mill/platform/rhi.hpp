#pragma once

#include "mill/core/base.hpp"

#include <glm/ext/vector_float4.hpp>

namespace mill::rhi
{

    /*
        A `Screen` is synonymous to a window or render surface, and typically refers to its back-buffers.
        A `View` is synonymous to a off-screen Render Target.
    */

#pragma region Core

    bool initialise();
    void shutdown();

    /* Should be called at the start of a frame before any contexts calls begin. */
    void begin_frame();
    /*
        Should be called at the end of a frame once any context calls have been completed.
        This will submit all contexts that have been recorded to.
    */
    void end_frame();

#pragma endregion Core

#pragma region Contexts

    void begin_contex(u64 context);
    void end_context(u64 context);

    void begin_view(u64 context, u64 view, const glm::vec4& clear_color = { 1, 1, 1, 1 }, f32 clear_depth = 1.0f);
    void end_view(u64 context, u64 view);

    void set_pipeline(u64 context);

    void set_index_buffer(u64 context);
    void set_vertex_buffer(u64 context);

    void draw(u64 context, u32 vertex_count);

    void blit_to_screen(u64 context, u64 screen, u64 view);

#pragma endregion Contexts

#pragma region Resources

    /* Assigns a window handle to a screen. Usually called as initialisation stage. */
    void assign_screen(u64 screen, void* window_handle);

    /* Update a screens back-buffer size. */
    void reset_screen(u64 screen, u32 width, u32 height, bool vsync);

    void reset_view(u64 view, u32 width, u32 height);

#pragma endregion Resources

}