#include "mill/core/engine.hpp"

#include "mill/core/base.hpp"
#include "platform/windowing.hpp"

#include <chrono>
using namespace std::chrono;

namespace mill
{
    static Engine* s_engine = nullptr;

    struct Engine::Pimpl
    {
        Application* app = nullptr;
        bool isRunning = true;

        f32 deltaTime = 0.0;

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

        m_pimpl->window = platform::create_window();
        m_pimpl->window->init(1600, 900, "Mill Engine"); // #TODO: Try get default window size/mode from TOML config file 
    }

    void Engine::shutdown()
    {
        LOG_INFO("Engine - Shutting down...");
    }

}
