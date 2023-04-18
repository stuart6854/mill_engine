#include "mill/scene/scene_manager.hpp"

#include "mill/scene/scene.hpp"

namespace mill
{
    SceneManager::SceneManager() {}

    void SceneManager::initialise()
    {
        load_scene();
    }

    void SceneManager::shutdown()
    {
        m_activeScene->on_unload();
        m_activeScene = nullptr;
    }

    void SceneManager::tick(f32 delta_time)
    {
        ASSERT(m_activeScene);

        m_activeScene->tick(delta_time);
    }

    void SceneManager::load_scene()
    {
        if (m_activeScene)
        {
            m_activeScene->on_unload();
            m_activeScene = nullptr;
        }

        m_activeScene = CreateShared<Scene>();
        m_activeScene->on_load();
    }

    auto SceneManager::get_active_scene() -> Scene&
    {
        ASSERT(m_activeScene);
        return *m_activeScene;
    }

}
