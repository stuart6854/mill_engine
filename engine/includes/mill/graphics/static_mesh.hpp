#pragma once

#include "mill/core/base.hpp"
#include "static_vertex.hpp"
#include "rhi/resources/rhi_buffer.hpp"
#include "mill/resources/resource.hpp"

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

#include <vector>

namespace mill
{
    class StaticMesh : public Resource
    {
    public:
        struct Submesh
        {
            u32 indexOffset{};
            u32 indexCount{};
            u32 vertexOffset{};
            u32 vertexCount{};
            u32 materialIndex{};
        };

        explicit StaticMesh() = default;
        ~StaticMesh() = default;

        void set_vertices(const std::vector<StaticVertex>& vertices);
        void set_triangles(const std::vector<u16>& triangles);
        void set_submeshes(const std::vector<Submesh>& submeshes);

        void apply();

        /* Getters */

        auto get_vertices() const -> const std::vector<StaticVertex>&;
        auto get_indices() const -> const std::vector<u16>&;
        auto get_submeshes() const -> const std::vector<Submesh>&;

    private:
        std::vector<StaticVertex> m_vertices{};
        std::vector<u16> m_triangles{};
        // TODO: Materials
        std::vector<Submesh> m_submeshes{};

        rhi::HandleBuffer m_indexBuffer{};
        rhi::HandleBuffer m_vertexBuffer{};
    };
}