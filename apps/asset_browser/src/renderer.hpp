#pragma once

#include <mill/mill.hpp>

namespace mill::asset_browser
{
    class Renderer
    {
    public:
        Renderer(u64 view_id);
        ~Renderer() = default;

        void initialise();
        void shutdown();

        auto render(u64 context_id) -> u64;

    private:
        u64 m_viewId{};
    };
}