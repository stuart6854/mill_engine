#pragma once

#include "base.hpp"

namespace mill
{
    class Events;
    class WindowInterface;
    class RendererInterface;
    class InputInterface;
    class ResourceManager;
    class Application;

    class Engine
    {
    public:
        static auto get() -> Engine*;

        Engine();
        ~Engine();

        void run(Application* app);

        void quit();

        /* Getters */

        auto get_events() const -> Events&;
        auto get_window() const -> WindowInterface*;
        auto get_renderer() const -> RendererInterface*;
        auto get_input() const -> InputInterface*;
        auto get_resources() const -> ResourceManager*;

    private:
        void initialise();
        void shutdown();

        void load_config();

    private:
        struct Pimpl;
        Owned<Pimpl> m_pimpl = nullptr;
    };
}