#include "mill/graphics/static_mesh.hpp"

#include "mill/graphics/rhi/resources/rhi_buffer.hpp"

namespace mill
{
    void StaticMesh::set_vertices(const std::vector<StaticVertex>& vertices)
    {
        m_vertices = vertices;
    }

    void StaticMesh::set_triangles(const std::vector<u16>& triangles)
    {
        m_triangles = triangles;
    }

    void StaticMesh::set_submeshes(const std::vector<Submesh>& submeshes)
    {
        m_submeshes = submeshes;
    }

    void StaticMesh::apply()
    {
        // Index Buffer
        {
#if 0
            if (m_indexBuffer)
                rhi::destroy_buffer(m_indexBuffer);
#endif

            rhi::BufferDescription buffer_desc{
                .size = sizeof(u16) * m_triangles.size(),
                .usage = rhi::BufferUsage::eIndexBuffer,
                .memoryUsage = rhi::MemoryUsage::eDeviceHostVisble,
            };
            m_indexBuffer = rhi::create_buffer(buffer_desc);
            rhi::write_buffer(m_indexBuffer, 0, buffer_desc.size, m_triangles.data());
        }
        m_indexCount = CAST_U32(m_triangles.size());

        // Vertex Buffer
        {
#if 0 
            if (m_indexBuffer) 
                rhi::destroy_buffer(m_indexBuffer);
#endif

            rhi::BufferDescription buffer_desc{
                .size = sizeof(StaticVertex) * m_vertices.size(),
                .usage = rhi::BufferUsage::eVertexBuffer,
                .memoryUsage = rhi::MemoryUsage::eDeviceHostVisble,
            };
            m_vertexBuffer = rhi::create_buffer(buffer_desc);
            rhi::write_buffer(m_vertexBuffer, 0, buffer_desc.size, m_vertices.data());
        }
    }

    auto StaticMesh::get_vertices() const -> const std::vector<StaticVertex>&
    {
        return m_vertices;
    }

    auto StaticMesh::get_indices() const -> const std::vector<u16>&
    {
        return m_triangles;
    }

    auto StaticMesh::get_submeshes() const -> const std::vector<Submesh>&
    {
        return m_submeshes;
    }

    auto StaticMesh::get_index_count() const -> u32
    {
        return m_indexCount;
    };

    auto StaticMesh::get_index_buffer() const -> rhi::HandleBuffer
    {
        return m_indexBuffer;
    }

    auto StaticMesh::get_vertex_buffer() const -> rhi::HandleBuffer
    {
        return m_vertexBuffer;
    }

}