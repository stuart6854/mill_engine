#pragma once

#include "mill/core/base.hpp"
#include "static_mesh.hpp"

#include <glm/ext/matrix_float4x4.hpp>

namespace mill
{
    struct RendererInit
    {
        void* window_handle = nullptr;
        u32 window_width{};
        u32 window_height{};
        bool window_vsync = true;
    };

    struct MeshInstance
    {
        // Mesh
        // Material
        // Transform
        // Render Flags - eg. AffectsShadows,
    };

    struct SceneInfo
    {
        // Camera - View, Proj
        glm::mat4 cameraProj{ 1.0f };
        glm::mat4 cameraView{ 1.0f };

        // MeshInstance[]
        // Lighting Info - Lights, Skybox, etc.
        // UI
    };

    /* Defines the High-Level Rendering API used by the Engine. */
    class RendererInterface
    {
    public:
        RendererInterface() = default;
        ~RendererInterface() = default;

        /* Commands */

        virtual void initialise(const RendererInit& init) = 0;
        virtual void shutdown() = 0;

        virtual void render(const SceneInfo& scene_info) = 0;

        virtual auto create_static_mesh() -> Owned<StaticMesh> = 0;
        virtual void destroy_static_mesh(StaticMesh* static_mesh) = 0;
    };

    namespace platform
    {
        auto create_renderer() -> Owned<RendererInterface>;
    }
}