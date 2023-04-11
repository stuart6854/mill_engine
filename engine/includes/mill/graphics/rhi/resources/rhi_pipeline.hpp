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
        // Format format{ Format::eUndefined };
    };

    enum class PrimitiveTopology : u8
    {
        eNone,
        ePoints,
        eLines,
        eTriangles,
    };

    struct PipelineVertexInputState
    {
        std::vector<VertexAttribute> attributes{};
        PrimitiveTopology topology{ PrimitiveTopology::eNone };

        auto get_hash() const -> hasht;
    };

    void set_pipeline_vertex_input_state(u64 context, const PipelineVertexInputState& state);

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

#pragma endregion
}