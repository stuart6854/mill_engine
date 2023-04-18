#pragma once

#include "mill/core/base.hpp"
#include "../rhi_resource.hpp"

#include <string>
#include <vector>

namespace mill::rhi
{
    struct VertexAttribute
    {
        std::string name{};
        Format format{ Format::eUndefined };
    };

    enum class PrimitiveTopology : u8
    {
        ePoints,
        eLines,
        eTriangles,
    };

    enum class FillMode : u8
    {
        eFill,
        eWireframe,
    };

    enum class CullMode : u8
    {
        eNone,
        eBack,
        eFront,
        eFrontAndBack,
    };

    enum class FrontFace
    {
        eClockwise,
        eCounterClockwise,
    };

    struct PipelineVertexInputState
    {
        std::vector<VertexAttribute> attributes{};
        PrimitiveTopology topology{ PrimitiveTopology::eTriangles };
    };

    struct PipelinePreRasterisationState
    {
        std::vector<u32> vertexSpirv{};
        FillMode fillMode{ FillMode::eFill };
        CullMode cullMode{ CullMode::eNone };
        FrontFace frontFace{ FrontFace::eClockwise };
        f32 lineWidth = 1.0f;
    };

    struct PipelineFragmentStageState
    {
        std::vector<u32> fragmentSpirv{};
        bool depthTest = false;
        bool stencilTest = false;
    };

    struct PipelineFragmentOutputState
    {
        // #TODO: Should I allow for attachments to be chosen? Would required change to views.
        // std::vector<Format> colorAttachmentFormats{};
        // Format depthAttachmentFormat{ Format::eUndefined };
        bool enableColorBlend{ false };
    };

    struct PipelineDescription
    {
        PipelineVertexInputState vertexInputState{};
        PipelinePreRasterisationState preRasterisationState{};
        PipelineFragmentStageState fragmentStageState{};
        PipelineFragmentOutputState fragmentOutputState{};
    };

    auto create_pipeline(const PipelineDescription& description) -> u64;

}