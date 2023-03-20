#include "mesh_exporter.hpp"

#include <mill/mill.hpp>
#include <mill/resources/resource_factory.hpp>

namespace mill::asset_browser
{
    void export_static_mesh(StaticMesh& mesh, const std::string& filename)
    {
        BinaryWriter writer(filename);

        // Resource Type Header
        writer.write_u8(g_StaticMeshHeader[0]);
        writer.write_u8(g_StaticMeshHeader[1]);
        writer.write_u8(g_StaticMeshHeader[2]);

        // Format version
        u16 formatVersion = 0;
        writer.write_u16(formatVersion);

        // Resource Id
        const u64 resourceId = 1998;
        writer.write_u64(resourceId);

        // Vertices
        const auto& vertices = mesh.get_vertices();
        writer.write_u64(vertices.size());
        for (const auto& vertex : vertices)
        {
            writer.write_f32(vertex.position.x);
            writer.write_f32(vertex.position.y);
            writer.write_f32(vertex.position.z);

            writer.write_f32(vertex.texCoord.x);
            writer.write_f32(vertex.texCoord.y);

            writer.write_f32(vertex.color.x);
            writer.write_f32(vertex.color.y);
            writer.write_f32(vertex.color.z);
        }

        // Triangles
        const auto& triangles = mesh.get_indices();
        writer.write_u64(triangles.size());
        for (auto index : triangles)
        {
            writer.write_u16(index);
        }

        // Submeshes
        const auto& submeshes = mesh.get_submeshes();
        writer.write_u64(submeshes.size());
        for (const auto& submesh : submeshes)
        {
            writer.write_u32(submesh.indexOffset);
            writer.write_u32(submesh.indexCount);
            writer.write_u32(submesh.vertexOffset);
            writer.write_u32(submesh.vertexCount);
            writer.write_u32(submesh.materialIndex);
        }
    }

}
