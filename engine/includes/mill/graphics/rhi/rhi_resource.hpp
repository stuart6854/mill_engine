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

    using HandleResourceSet = u64;
    auto create_resource_set(const ResourceSetDescription& description) -> HandleResourceSet;

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

    using HandlePipeline = u64;
    auto create_pipeline(const PipelineDescription& description) -> HandlePipeline;

    // auto create_shader(const std::vector<u32>& compiled_code) -> HandleShader?; // Shader are responsible for shader

#endif

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

    using HandleBuffer = u64;
    auto create_buffer(const BufferDescription& description) -> HandleBuffer;

    void write_buffer(HandleBuffer buffer, u64 offset, u64 size, const void* data);
}

#include "resources/rhi_pipeline.hpp"