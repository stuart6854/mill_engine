#pragma once

#include "mill/core/base.hpp"
#include "mill/resources/resource.hpp"
#include "mill/utility/signal.hpp"

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

#include <vector>

namespace mill
{
    struct StaticVertex
    {
        glm::vec3 position{};
        glm::vec2 texCoord{};
        glm::vec4 color{};
    };

    class StaticMesh : public Resource
    {
    public:
        Signal<StaticMesh*> cb_on_data_changed{};
        Signal<StaticMesh*> cb_on_destroyed{};

        explicit StaticMesh() = default;
        ~StaticMesh();

        void set_vertices(const std::vector<StaticVertex>& vertices);
        void set_triangles(const std::vector<u16>& triangles);

        void apply();

        /* Getters */

        auto get_vertices() const -> const std::vector<StaticVertex>&;
        auto get_indices() const -> const std::vector<u16>&;

    private:
        std::vector<StaticVertex> m_vertices{};
        std::vector<u16> m_triangles{};
    };
}