#pragma once

#include "assets/asset_registry.hpp"
#include "views/asset_browser_view.hpp"
#include "views/asset_settings_view.hpp"
#include "views/asset_preview_view.hpp"
#include "renderer.hpp"

#include <mill/mill.hpp>

#include <filesystem>

namespace mill::asset_browser
{
    namespace fs = std::filesystem;

    class AssetBrowserApp : public mill::Application
    {
    public:
        void initialise() override;
        void shutdown() override;

        void update(f32 delta_time) override;

        void reload_project();

        void import_assets();
        void import_asset(const fs::path& asset_filename, const fs::path& target_dir);

    private:
        void event_callback(const Event& event);

        auto gather_scene_info() -> SceneRenderInfo;

        void init_imgui();
        void shutdown_imgui();

        void open_project();
        void open_project(const fs::path& project_dir);

        void scan_for_and_register_assets(const fs::path& dir_to_scan);

    private:
        platform::HandleWindow m_windowHandle{ nullptr };
        Owned<SceneRenderer> m_sceneRenderer{ nullptr };
        Owned<Renderer> m_renderer{};

        const u64 g_PrimaryScreenId = "primary_screen"_hs;
        const u64 g_SceneViewId = "scene_view"_hs;
        const u64 g_MainViewId = "main_view"_hs;

        AssetRegistry m_assetRegistry{};
        AssetBrowserView m_assetBrowserView{};
        AssetSettingView m_assetSettingsView{};
        AssetPreviewView m_assetPreviewView{};

        fs::path m_projectDir{};
    };
}