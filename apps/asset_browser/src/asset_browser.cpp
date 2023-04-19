#include "asset_browser.hpp"

#include "renderer.hpp"
#include "assets/assets.hpp"
#include "assets/mesh_importer.hpp"
#include "assets/mesh_exporter.hpp"

#include <filesystem>

namespace mill::asset_browser
{
    void handle_static_mesh(const std::string& filename)
    {
        auto mesh = import_static_mesh(filename);
        export_static_mesh(*mesh, filename + ".bin");
    }

    void AssetBrowserApp::initialise()
    {
        auto& events = Engine::get()->get_events();
        events.subscribe([this](const Event& event) { event_callback(event); });

        platform::WindowInfo window_info{};
        m_windowHandle = platform::create_window(window_info);

        rhi::assign_screen(g_PrimaryScreenId, platform::get_raw_window_handle(m_windowHandle));
        rhi::reset_screen(g_PrimaryScreenId, 1600, 900, true);

        m_renderer = CreateOwned<Renderer>(g_MainViewId);
        m_renderer->initialise();

        // Load and export all assets in asset directory
        for (auto& dir_entry : std::filesystem::recursive_directory_iterator(g_AssetPath))
        {
            if (!dir_entry.is_regular_file())
            {
                continue;
            }

            const auto filePath = dir_entry.path();
            const auto ext = filePath.extension().string();
            if (!g_AssetTypeMap.contains(ext))
            {
                continue;
            }

            LOG_INFO("AssetBrowser - Importing asset <{}>", filePath.string());

            const auto assetType = g_AssetTypeMap.at(ext);
            switch (assetType)
            {
                case AssetType::eMesh: handle_static_mesh(filePath.string()); break;
                default: break;
            }
        }
    }

    void AssetBrowserApp::shutdown()
    {
        m_renderer->shutdown();
        m_renderer = nullptr;

        platform::destroy_window(m_windowHandle);
    }

    void AssetBrowserApp::update()
    {
        rhi::begin_frame();
        {
            const static auto ContextId = "main_render_context"_hs;
            rhi::begin_context(ContextId);
            {
                auto view_id = m_renderer->render(ContextId);

                rhi::blit_to_screen(ContextId, g_PrimaryScreenId, view_id);
            }
            rhi::end_context(ContextId);
        }
        rhi::end_frame();
    }

    void AssetBrowserApp::event_callback(const Event& event)
    {
        if (event.type == EventType::eWindowClose)
        {
            const platform::HandleWindow windowContext = event.context;
            if (windowContext == m_windowHandle)
            {
                Engine::get()->quit();
            }
        }
        else if (event.type == EventType::eWindowSize)
        {
            const platform::HandleWindow windowContext = event.context;
            if (windowContext == m_windowHandle)
            {
                u32 width = event.data.u32[0];
                u32 height = event.data.u32[1];
                rhi::reset_screen(g_PrimaryScreenId, width, height, true);
                rhi::reset_view(g_MainViewId, width, height);
            }
        }
    }

}
