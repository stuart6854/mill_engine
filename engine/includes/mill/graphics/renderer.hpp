#pragma once

#include "mill/core/base.hpp"

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
        // MeshInstance[]
        // Lighting Info - Lights, Skybox, etc.
        // UI
    };

    class RendererInterface
    {
    public:
        RendererInterface() = default;
        ~RendererInterface() = default;

        /* Commands */

        virtual void inititialise(const RendererInit& init) = 0;
        virtual void shutdown() = 0;

        virtual void render(const SceneInfo& scene_info) = 0;
    };

    namespace platform
    {
        auto create_renderer() -> Owned<RendererInterface>;
    }
}