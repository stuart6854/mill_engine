#pragma once

#include "renderer.hpp"
#include "views/asset_browser_view.hpp"

#include <mill/mill.hpp>

#include <filesystem>

namespace mill::asset_browser
{
    class AssetBrowserApp : public mill::Application
    {
    public:
        void initialise() override;
        void shutdown() override;

        void update(f32 delta_time) override;

        void reload_project();

    private:
        void event_callback(const Event& event);

        void init_imgui();
        void shutdown_imgui();

        void open_project();
        void open_project(const std::filesystem::path& project_dir);

    private:
        platform::HandleWindow m_windowHandle{ nullptr };
        Owned<Renderer> m_renderer{};

        const u64 g_PrimaryScreenId = "primary_screen"_hs;
        const u64 g_MainViewId = "main_view"_hs;

        AssetBrowserView m_assetBrowserView{};

        std::filesystem::path m_projectDir{};
    };
}