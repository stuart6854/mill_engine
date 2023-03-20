#include "mesh_importer.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace mill::asset_browser
{
    constexpr auto g_StaticMeshImportFlags = aiProcessPreset_TargetRealtime_MaxQuality;

    void process_mesh(const aiMesh* mesh,
                      std::vector<StaticVertex>& vertices,
                      std::vector<u16>& triangles,
                      std::vector<StaticMesh::Submesh>& submeshes)
    {
        auto& submesh = submeshes.emplace_back();
        submesh.indexOffset = static_cast<u32>(triangles.size());
        submesh.indexCount = mesh->mNumFaces * 3;
        submesh.vertexOffset = static_cast<u32>(vertices.size());
        submesh.vertexCount = mesh->mNumVertices;
        submesh.materialIndex = mesh->mMaterialIndex;

        vertices.reserve(vertices.size() + submesh.vertexCount);
        for (sizet i = 0; i < mesh->mNumVertices; ++i)
        {
            auto& out_vertex = vertices.emplace_back();

            const auto& in_position = mesh->mVertices[i];
            out_vertex.position = { in_position.x, in_position.y, in_position.z };

            if (mesh->HasTextureCoords(0))
            {
                const auto& in_texCoord = mesh->mTextureCoords[0][i];
                out_vertex.texCoord = { in_texCoord.x, in_texCoord.y };
            }

            if (mesh->HasVertexColors(0))
            {
                const auto& in_color = mesh->mColors[0][i];
                out_vertex.color = { in_color.r, in_color.g, in_color.b, in_color.a };
            }
        }

        triangles.reserve(triangles.size() + submesh.indexCount);
        for (sizet i = 0; i < mesh->mNumFaces; ++i)
        {
            const auto& face = mesh->mFaces[i];
            ASSERT(face.mNumIndices == 3);
            triangles.push_back(static_cast<u16>(face.mIndices[0]));
            triangles.push_back(static_cast<u16>(face.mIndices[1]));
            triangles.push_back(static_cast<u16>(face.mIndices[2]));
        }
    }

    void process_node(const aiNode* node,
                      const aiScene* scene,
                      std::vector<StaticVertex>& vertices,
                      std::vector<u16>& triangles,
                      std::vector<StaticMesh::Submesh>& submeshes)
    {
        for (sizet i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[i];
            process_mesh(mesh, vertices, triangles, submeshes);
        }

        for (sizet i = 0; i < node->mNumChildren; ++i)
        {
            process_node(node->mChildren[i], scene, vertices, triangles, submeshes);
        }
    }

    auto import_static_mesh(const std::string& filename) -> Owned<StaticMesh>
    {
        Assimp::Importer importer{};
        const auto* scene = importer.ReadFile(filename, g_StaticMeshImportFlags);
        if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
        {
            LOG_ERROR("AssetBrowser - StaticMeshImporter - Failed to load file <{}>: {}", filename, importer.GetErrorString());
            return nullptr;
        }

        std::vector<StaticVertex> vertices{};
        std::vector<u16> triangles{};
        std::vector<StaticMesh::Submesh> submeshes{};

        process_node(scene->mRootNode, scene, vertices, triangles, submeshes);

        auto static_mesh = Engine::get()->get_renderer()->create_static_mesh();
        static_mesh->set_submeshes(submeshes);
        static_mesh->set_vertices(vertices);
        static_mesh->set_triangles(triangles);
        static_mesh->apply();
        return static_mesh;
    }

}
