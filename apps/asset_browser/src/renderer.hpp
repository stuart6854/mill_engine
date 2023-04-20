#pragma once

#include <mill/mill.hpp>

#include <imgui.h>

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
        void render_draw_data(const ImDrawData* draw_data);

    private:
        u64 m_fontTexture{};
        u64 m_viewId{};
    };
}