#include "mill/core/engine.hpp"

#include "mill/core/base.hpp"
#include "platform/windowing.hpp"

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

        auto lastFrameTime = high_resolution_clock::now();
        while (m_pimpl->isRunning)
        {
            auto now = high_resolution_clock::now();
            m_pimpl->deltaTime = duration_cast<milliseconds>(now - lastFrameTime).count() / 1000.0f;

            m_pimpl->window->poll_events();
        }

        shutdown();
    }

    void Engine::quit()
    {
        m_pimpl->isRunning = false;
    }

    void Engine::initialise()
    {
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
    }

    void Engine::shutdown()
    {
        LOG_INFO("Engine - Shutting down...");
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
