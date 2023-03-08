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

        virtual void update() {}
    };

    extern auto create_application() -> Application*;
}