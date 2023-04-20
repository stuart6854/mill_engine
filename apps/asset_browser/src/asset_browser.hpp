#pragma once

#include "renderer.hpp"

#include <mill/mill.hpp>

namespace mill::asset_browser
{
    class AssetBrowserApp : public mill::Application
    {
    public:
        void initialise() override;
        void shutdown() override;

        void update(f32 delta_time) override;

    private:
        void event_callback(const Event& event);

        void init_imgui();
        void shutdown_imgui();

    private:
        platform::HandleWindow m_windowHandle{ nullptr };
        Owned<Renderer> m_renderer{};

        const u64 g_PrimaryScreenId = "primary_screen"_hs;
        const u64 g_MainViewId = "main_view"_hs;
    };
}