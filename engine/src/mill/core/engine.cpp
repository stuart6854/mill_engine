#include "mill/core/engine.hpp"

#include "mill/core/base.hpp"
#include "mill/events/events.hpp"
#include "mill/platform/platform_interface.hpp"
#include "mill/core/application.hpp"
#include "platform/windowing.hpp"
#include "mill/input/input.hpp"
#include "mill/resources/resource_manager.hpp"
#include "mill/scene/scene_manager.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <toml.hpp>

#include <chrono>
using namespace std::chrono;
#include <fstream>
#include <filesystem>

namespace mill
{
#define INIT_SYSTEM(_system, _assign_value, ...) \
    m_pimpl->_system = _assign_value;            \
    m_pimpl->_system->initialise(__VA_ARGS__)

#define SHUTDOWN_SYSTEM(_system)  \
    m_pimpl->_system->shutdown(); \
    m_pimpl->_system = nullptr

    namespace
    {
        auto create_default_config() -> toml::table
        {
            toml::table config{
                { "window",
                  toml::table{
                      { "resolution", toml::array{ 1080, 720 } },
                      { "mode", 0 },
                  } },
            };

            return config;
        }
    }

    static Engine* s_engine = nullptr;

    struct Engine::Pimpl
    {
        Application* app = nullptr;
        bool isRunning = true;

        f32 deltaTime = 0.0;

        // Engine Config
        toml::table config{};

        /* Systems */

        Events events{};

        Owned<WindowInterface> window = nullptr;
        Owned<InputInterface> input = nullptr;
        Owned<ResourceManager> resources = nullptr;
        Owned<SceneManager> sceneManager{ nullptr };

        glm::vec3 cameraPosition{ 0, 0, -5.0f };
        glm::vec3 cameraDirection{ 0, 0, 1 };
    };

    auto Engine::get() -> Engine*
    {
        return s_engine;
    }

    Engine::Engine() : m_pimpl(new Pimpl)
    {
        s_engine = this;
    }

    Engine::~Engine()
    {
        s_engine = nullptr;
    }

    void Engine::run(Application* app)
    {
        LOG_INFO("Mill Engine");

        m_pimpl->app = app;

        initialise();

        // auto handle = m_pimpl->resources->get_handle(1998, true);
        // auto* mesh = handle.As<StaticMesh>();
        // UNUSED(mesh);

        using clock = high_resolution_clock;
        auto lastFrameTime = clock::now();
        while (m_pimpl->isRunning)
        {
            platform::platform_pump_messages();
            m_pimpl->events.flush_queue();

            auto now = clock::now();
            u64 ms = duration_cast<milliseconds>(now - lastFrameTime).count();
            m_pimpl->deltaTime = static_cast<f32>(static_cast<f64>(ms) / 1000.0f);
            lastFrameTime = now;

            m_pimpl->input->new_frame();
            // m_pimpl->window->poll_events();

            m_pimpl->sceneManager->tick(m_pimpl->deltaTime);

            // Print delta time
            if (m_pimpl->input->on_key_held(KeyCodes::F1))
            {
                LOG_DEBUG("Delta Time - {}ms / {:.3f}s", ms, m_pimpl->deltaTime);
            }

            // Camera Controls
            if (m_pimpl->input->on_mouse_btn_held(MouseButtonCodes::MouseRight))
            {
                static const glm::vec3 world_up_dir = { 0, 1, 0 };
                static const f32 s_CameraSpeed = 5.0f;

                glm::vec3 movement{};
                if (m_pimpl->input->on_key_held(KeyCodes::W))
                {
                    movement += m_pimpl->cameraDirection;
                }
                if (m_pimpl->input->on_key_held(KeyCodes::S))
                {
                    movement += -m_pimpl->cameraDirection;
                }
                const glm::vec3 rel_right_dir = -glm::cross(m_pimpl->cameraDirection, world_up_dir);
                if (m_pimpl->input->on_key_held(KeyCodes::D))
                {
                    movement += rel_right_dir;
                }
                if (m_pimpl->input->on_key_held(KeyCodes::A))
                {
                    movement += -rel_right_dir;
                }
                const auto rel_up_dir = -glm::cross(rel_right_dir, m_pimpl->cameraDirection);
                if (m_pimpl->input->on_key_held(KeyCodes::Space))
                {
                    movement += rel_up_dir;
                }
                if (m_pimpl->input->on_key_held(KeyCodes::LeftAlt))
                {
                    movement += -rel_up_dir;
                }
                if (glm::length(movement) > 0.0f)
                {
                    m_pimpl->cameraPosition += glm::normalize(movement) * s_CameraSpeed * m_pimpl->deltaTime;
                    // LOG_DEBUG("{}, {}, {}", m_pimpl->cameraPosition.x, m_pimpl->cameraPosition.y, m_pimpl->cameraPosition.z);
                }

                static const f32 s_CameraSensitivity = 1.0f;
                const auto cursor_delta = m_pimpl->input->get_cursor_delta();
                if (glm::length(cursor_delta) > 0.0f)
                {
                    const f32 speed = s_CameraSensitivity * m_pimpl->deltaTime;
                    m_pimpl->cameraDirection = glm::rotate(m_pimpl->cameraDirection, cursor_delta.y * speed, rel_right_dir);
                    m_pimpl->cameraDirection = glm::rotate(m_pimpl->cameraDirection, cursor_delta.x * speed, rel_up_dir);
                }
            }

            /* const f32 aspect_ratio =
                static_cast<f32>(m_pimpl->window->get_resolution().x) / static_cast<f32>(m_pimpl->window->get_resolution().y);
            SceneInfo scene_info{};
            scene_info.cameraProj = glm::perspective(glm::radians(60.0f), aspect_ratio, 0.1f, 100.0f);
            scene_info.cameraView =
                glm::lookAt(m_pimpl->cameraPosition, m_pimpl->cameraPosition + m_pimpl->cameraDirection, glm::vec3(0, 1, 0));

            auto& mesh_instance = scene_info.meshInstances.emplace_back();
            // mesh_instance.mesh = handle.As<StaticMesh>();
            mesh_instance.transform = glm::mat4(1.0f);*/

            if (m_pimpl->app != nullptr)
            {
                m_pimpl->app->update();
            }
        }

        shutdown();
    }

    void Engine::quit()
    {
        m_pimpl->isRunning = false;
    }

    auto Engine::get_events() const -> Events&
    {
        return m_pimpl->events;
    }

    auto Engine::get_window() const -> WindowInterface*
    {
        return m_pimpl->window.get();
    }

    auto Engine::get_input() const -> InputInterface*
    {
        return m_pimpl->input.get();
    }

    auto Engine::get_resources() const -> ResourceManager*
    {
        return m_pimpl->resources.get();
    }

    void Engine::initialise()
    {
#if MILL_DEBUG
        spdlog::set_level(spdlog::level::trace);
#endif

        LOG_INFO("Engine - Initialising...");

        load_config();

        // auto toml_window_size = m_pimpl->config["window"]["resolution"].as_array();
        //  auto window_width = static_cast<u32>(static_cast<::mill::i64>(*toml_window_size->get(0)->as_integer()));
        //  auto window_height = static_cast<u32>(static_cast<::mill::i64>(*toml_window_size->get(1)->as_integer()));

        platform::platform_initialise();

        INIT_SYSTEM(input, CreateOwned<InputDefault>());
        /*m_pimpl->window->cb_on_input_keyboard_key.connect([this](i32 key, bool is_down)
                                                          { m_pimpl->input->set_key(static_cast<KeyCodes>(key), is_down); });
        m_pimpl->window->cb_on_input_mouse_btn.connect([this](i32 btn, bool is_down)
                                                       { m_pimpl->input->set_mouse_btn(static_cast<MouseButtonCodes>(btn), is_down); });
        m_pimpl->window->cb_on_input_cursor_pos.connect([this](glm::vec2 pos) { m_pimpl->input->set_cursor_pos(pos); });*/

        ResourceManagerInit resource_manager_init{};
        INIT_SYSTEM(resources, CreateOwned<ResourceManager>(), resource_manager_init);

        /*m_pimpl->resources->register_resource_type<StaticMesh>(ResourceType_StaticMesh,
                                                               std::move(CreateOwned<StaticMeshFactory>(m_pimpl->renderer.get())));*/

        INIT_SYSTEM(sceneManager, CreateOwned<SceneManager>());

        m_pimpl->app->initialise();
    }

    void Engine::shutdown()
    {
        LOG_INFO("Engine - Shutting down...");

        m_pimpl->app->shutdown();

        SHUTDOWN_SYSTEM(sceneManager);
        SHUTDOWN_SYSTEM(resources);
        SHUTDOWN_SYSTEM(input);

        platform::platform_shutdown();
    }

    void Engine::load_config()
    {
        // Load engine config
        const std::filesystem::path config_file = "../../config/engine.toml";
        if (std::filesystem::exists(config_file))
        {
            m_pimpl->config = toml::parse_file(config_file.string());
        }
        else
        {
            LOG_WARN("Engine - No engine config file. Creating a default one...");
            m_pimpl->config = create_default_config();

            std::filesystem::create_directory(config_file.parent_path());
            std::ofstream file(config_file, std::ios::trunc);
            file << m_pimpl->config;
            file.close();
        }
    }
}
