#pragma once

#include "mill/core/base.hpp"

namespace mill
{
    struct RendererInit
    {
        void* window_handle = nullptr;
        u32 window_width{};
        u32 window_height{};
        bool window_vsync = true;
    };

    class RendererInterface
    {
    public:
        RendererInterface() = default;
        ~RendererInterface() = default;

        /* Commands */

        virtual void inititialise(const RendererInit& init) = 0;
        virtual void shutdown() = 0;

        virtual void wait_and_begin_frame() = 0;
        virtual void submit_and_present() = 0;
    };

    namespace platform
    {
        auto create_renderer() -> Owned<RendererInterface>;
    }
}