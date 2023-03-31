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

    using HandlePipeline = u64;
    using HandleBuffer = u64;

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

#pragma endregion Contexts

#pragma region Resources

    /* Assigns a window handle to a screen. Usually called as initialisation stage. */
    void assign_screen(u64 screen, void* window_handle);

    /* Update a screens back-buffer size. */
    void reset_screen(u64 screen, u32 width, u32 height, bool vsync);

    void reset_view(u64 view, u32 width, u32 height);

    enum class Format
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

    struct VertexAttribute
    {
        std::string name{};
        Format format{};
    };

    enum class PrimitiveTopology
    {
        ePoints,
        eLines,
        eTriangles,
    };

    struct BlendDescription
    {
    };

    enum class FillMode
    {
        eWireframe,
        eSolid,
    };

    enum class CullMode
    {
        eNone,
        eFront,
        eBack,
    };

    enum class WindingOrder
    {
        eClockwise,
        eCounterClockwise,
    };

    struct RasterizerDescription
    {
        FillMode fillMode{ FillMode::eSolid };
        CullMode cullMode{ CullMode::eNone };
        WindingOrder windingOrder{ WindingOrder::eClockwise };
        f32 lineWidth{ 1.0f };
    };

    struct DepthStencilDescription
    {
        bool depthEnabled{ false };
        bool stencilEnabled{ false };
    };

    struct PipelineDescription
    {
        std::vector<u32> vs{};
        std::vector<u32> fs{};

        std::vector<VertexAttribute> vertexAttributes{};

        PrimitiveTopology topology{ PrimitiveTopology::eTriangles };
        std::vector<Format> colorTargets{};
        Format depthStencilTarget{ Format::eUndefined };

        BlendDescription blendDesc{};
        RasterizerDescription rasterizerDesc{};
        DepthStencilDescription depthStencilDesc{};
    };

    auto create_pipeline(const PipelineDescription& description) -> HandlePipeline;

    // auto create_shader(const std::vector<u32>& compiled_code) -> HandleShader?; // Shader are responsible for shader

    enum class BufferUsage
    {
        eVertexBuffer,
        eIndexBuffer,
        eUniformBuffer,
        eTransferSrc,
    };

    enum class MemoryUsage
    {
        eHost,
        eDevice,
        eDeviceHostVisble,
    };

    struct BufferDescription
    {
        u64 size{};
        BufferUsage usage{};
        MemoryUsage memoryUsage{};
    };

    auto create_buffer(const BufferDescription& description) -> HandleBuffer;

    void write_buffer(HandleBuffer buffer, u64 offset, u64 size, const void* data);

#pragma endregion Resources

#pragma region Utility

    auto compile_shader(const std::string_view& code) -> std::vector<u32>;

#pragma endregion Utility

}