#pragma once

#include "base.hpp"

namespace mill
{
    class Application;

    class Engine
    {
    public:
        static auto get() -> Engine*;

        Engine();
        ~Engine();

        void run(Application* app);

        void quit();

    private:
        void initialise();
        void shutdown();

    private:
        struct Pimpl;
        Owned<Pimpl> m_pimpl = nullptr;
    };
}