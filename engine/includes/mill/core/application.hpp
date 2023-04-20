#pragma once

namespace mill
{
    class Application
    {
    public:
        Application() = default;
        ~Application() = default;

        virtual void initialise() {}
        virtual void shutdown() {}

        virtual void update(f32 /*delta_time*/) {}
    };

    extern auto create_application() -> Application*;
}