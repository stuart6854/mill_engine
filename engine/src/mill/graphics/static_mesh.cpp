#include "mill/graphics/static_mesh.hpp"

namespace mill
{
    StaticMesh::~StaticMesh()
    {
        cb_on_destroyed.emit(this);
    }

    void StaticMesh::set_vertices(const std::vector<StaticVertex>& vertices)
    {
        m_vertices = vertices;
    }

    void StaticMesh::set_triangles(const std::vector<u16>& triangles)
    {
        m_triangles = triangles;
    }

    void StaticMesh::apply()
    {
        cb_on_data_changed.emit(this);
    }

    auto StaticMesh::get_vertices() const -> const std::vector<StaticVertex>&
    {
        return m_vertices;
    }

    auto StaticMesh::get_indices() const -> const std::vector<u16>&
    {
        return m_triangles;
    }

}