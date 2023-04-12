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

        auto get_hash() const -> hasht;
    };

    struct PipelinePreRasterisationState
    {
        std::vector<u32> vertexSpirv{};
        FillMode fillMode{ FillMode::eFill };
        CullMode cullMode{ CullMode::eNone };
        FrontFace frontFace{ FrontFace::eClockwise };
        f32 lineWidth = 1.0f;

        auto get_hash() const -> hasht;
    };

    struct PipelineFragmentStageState
    {
        std::vector<u32> fragmentSpirv{};
        bool depthTest = false;
        bool stencilTest = false;

        auto get_hash() const -> hasht;
    };

    struct PipelineFragmentOutputState
    {
        std::vector<Format> colorAttachmentFormats{};
        Format depthAttachmentFormat{ Format::eUndefined };
        bool enableColorBlend{ false };

        auto get_hash() const -> hasht;
    };

#pragma region Pipeline State Hash Functions

    inline auto PipelineVertexInputState::get_hash() const -> hasht
    {
        hasht hash{};

        hash_combine(hash, CAST_U32(attributes.size()));
        /* for (const auto& attribute : attributes)
        {
            hash_combine(hash, enum_to_underlying(attribute.format));
        } */
        hash_combine(hash, enum_to_underlying(topology));

        return hash;
    }

    inline auto PipelinePreRasterisationState::get_hash() const -> hasht
    {
        hasht hash{};

        hash_combine(hash, CAST_U32(vertexSpirv.size()));
        for (auto& value : vertexSpirv)
        {
            hash_combine(hash, value);
        }
        hash_combine(hash, enum_to_underlying(fillMode));
        hash_combine(hash, enum_to_underlying(cullMode));
        hash_combine(hash, enum_to_underlying(frontFace));

        return hash;
    }

    inline auto PipelineFragmentStageState::get_hash() const -> hasht
    {
        hasht hash{};

        hash_combine(hash, CAST_U32(fragmentSpirv.size()));
        for (auto& value : fragmentSpirv)
        {
            hash_combine(hash, value);
        }
        hash_combine(hash, depthTest);
        hash_combine(hash, stencilTest);

        return hash;
    }

    inline auto PipelineFragmentOutputState::get_hash() const -> hasht
    {
        hasht hash{};

        hash_combine(hash, enableColorBlend);

        return hash;
    }

#pragma endregion
}