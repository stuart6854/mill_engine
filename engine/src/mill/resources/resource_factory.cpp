#include "mill/resources/resource_factory.hpp"

#include "mill/io/binary_reader.hpp"

namespace mill
{
    auto StaticMeshFactory::load(const ResourceMetadata& metadata) -> Owned<Resource>
    {
        UNUSED(metadata);
#if 0
        BinaryReader reader(metadata.binaryFile);

        // File Header
        std::string header(3, ' ');
        header[0] = reader.read_u8();
        header[1] = reader.read_u8();
        header[2] = reader.read_u8();
        ASSERT(header == g_StaticMeshHeader);
        if (header != g_StaticMeshHeader)
        {
            LOG_ERROR("ResourceManager - StaticMeshFactory - Incorrect resource header!");
            return nullptr;
        }

        // Format Version
        u16 format_version = reader.read_u16();
        UNUSED(format_version);

        // Resource Id
        u64 resource_id = reader.read_u64();
        UNUSED(resource_id);

        // Vertices
        sizet vertex_count = reader.read_u64();
        std::vector<StaticVertex> vertices{};
        vertices.reserve(vertex_count);
        for (sizet i = 0; i < vertex_count; ++i)
        {
            auto& vertex = vertices.emplace_back();

            vertex.position.x = reader.read_f32();
            vertex.position.y = reader.read_f32();
            vertex.position.z = reader.read_f32();

            vertex.texCoord.x = reader.read_f32();
            vertex.texCoord.y = reader.read_f32();

            vertex.color.x = reader.read_f32();
            vertex.color.y = reader.read_f32();
            vertex.color.z = reader.read_f32();
        }

        // Triangles
        sizet triangle_count = reader.read_u64();
        std::vector<u16> triangles{};
        triangles.reserve(triangle_count);
        for (sizet i = 0; i < triangle_count; ++i)
        {
            triangles.push_back(reader.read_u16());
        }

        // Submeshes
        sizet submesh_count = reader.read_u64();
        std::vector<StaticMesh::Submesh> submeshes{};
        submeshes.reserve(submesh_count);
        for (sizet i = 0; i < submesh_count; ++i)
        {
            auto& submesh = submeshes.emplace_back();

            submesh.indexOffset = reader.read_u32();
            submesh.indexCount = reader.read_u32();
            submesh.vertexOffset = reader.read_u32();
            submesh.vertexCount = reader.read_u32();
            submesh.materialIndex = reader.read_u32();
        }

        auto static_mesh = m_renderer->create_static_mesh();
        static_mesh->set_vertices(vertices);
        static_mesh->set_triangles(triangles);
        static_mesh->set_submeshes(submeshes);
        static_mesh->apply();
        return std::move(static_mesh);
#endif
        return nullptr;
    }

}
