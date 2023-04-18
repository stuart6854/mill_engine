#pragma once

#include "mill/core/base.hpp"

namespace mill
{
    class Scene;

    class SceneManager
    {
    public:
        SceneManager();
        ~SceneManager() = default;

        void initialise();
        void shutdown();

        void tick(f32 delta_time);

        void load_scene();

        /* Getters */

        auto get_active_scene() -> Scene&;

    private:
        Shared<Scene> m_activeScene{};
    };
}