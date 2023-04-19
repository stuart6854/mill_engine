#include "mill/scene/scene_manager.hpp"

#include "mill/scene/scene.hpp"

namespace mill
{
    SceneManager::SceneManager() {}

    void SceneManager::initialise()
    {
        LOG_INFO("SceneManager - Initialising...");
        load_scene();
    }

    void SceneManager::shutdown()
    {
        LOG_INFO("SceneManager - Shutting down...");
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
            LOG_INFO("SceneManage - Unloading scene.");
            m_activeScene->on_unload();
            m_activeScene = nullptr;
        }

        m_activeScene = CreateShared<Scene>();
        LOG_INFO("SceneManager - Loading scene.");
        m_activeScene->on_load();
    }

    auto SceneManager::get_active_scene() -> Scene&
    {
        ASSERT(m_activeScene);
        return *m_activeScene;
    }

}
