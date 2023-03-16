#include "mill/core/engine.hpp"

#include "mill/core/base.hpp"
#include "platform/windowing.hpp"
#include "platform/graphics.hpp"
#include "mill/input/input.hpp"

#include <toml.hpp>

#include <chrono>
using namespace std::chrono;
#include <fstream>
#include <filesystem>

namespace mill
{
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

        Owned<WindowInterface> window = nullptr;
        Owned<RendererInterface> renderer = nullptr;
        Owned<InputInterface> input = nullptr;
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

        using clock = high_resolution_clock;
        auto lastFrameTime = clock::now();
        while (m_pimpl->isRunning)
        {
            auto now = clock::now();
            u64 ms = duration_cast<milliseconds>(now - lastFrameTime).count();
            m_pimpl->deltaTime = static_cast<f32>(static_cast<f64>(ms) / 1000.0f);
            lastFrameTime = now;

            m_pimpl->input->new_frame();
            m_pimpl->window->poll_events();

            // Print delta time
            if (m_pimpl->input->on_key_held(KeyCodes::F1))
            {
                LOG_DEBUG("Delta Time - {}ms / {:.3f}s", ms, m_pimpl->deltaTime);
            }

            SceneInfo scene_info{};
            m_pimpl->renderer->render(scene_info);
        }

        shutdown();
    }

    void Engine::quit()
    {
        m_pimpl->isRunning = false;
    }

    void Engine::initialise()
    {
#if MILL_DEBUG
        spdlog::set_level(spdlog::level::trace);
#endif

        LOG_INFO("Engine - Initialising...");

        load_config();

        auto toml_window_size = m_pimpl->config["window"]["resolution"].as_array();
        auto window_width = static_cast<u32>(static_cast<::mill::i64>(*toml_window_size->get(0)->as_integer()));
        auto window_height = static_cast<u32>(static_cast<::mill::i64>(*toml_window_size->get(1)->as_integer()));

        WindowInit window_init{
            window_width,
            window_height,
            "Mill Engine",
        };
        m_pimpl->window = platform::create_window();
        m_pimpl->window->init(window_init);
        m_pimpl->window->cb_on_window_close_requested.connect_member(this, &Engine::quit);

        RendererInit renderer_init{
            m_pimpl->window->get_platform_handle(),
            window_width,
            window_height,
        };
        m_pimpl->renderer = platform::create_renderer();
        m_pimpl->renderer->inititialise(renderer_init);

        m_pimpl->input = CreateOwned<InputDefault>();
        m_pimpl->input->init();
        m_pimpl->window->cb_on_input_keyboard_key.connect([this](i32 key, bool is_down)
                                                          { m_pimpl->input->set_key(static_cast<KeyCodes>(key), is_down); });
        m_pimpl->window->cb_on_input_mouse_btn.connect([this](i32 btn, bool is_down)
                                                       { m_pimpl->input->set_mouse_btn(static_cast<MouseButtonCodes>(btn), is_down); });
        m_pimpl->window->cb_on_input_cursor_pos.connect([this](glm::vec2 pos) { m_pimpl->input->set_cursor_pos(pos); });
    }

    void Engine::shutdown()
    {
        LOG_INFO("Engine - Shutting down...");

        m_pimpl->input->shutdown();
        m_pimpl->input = nullptr;

        m_pimpl->renderer->shutdown();
        m_pimpl->renderer = nullptr;

        m_pimpl->window->shutdown();
        m_pimpl->window = nullptr;
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
