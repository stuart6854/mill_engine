#include "asset_browser.hpp"

#include "renderer.hpp"
#include "assets/assets.hpp"
#include "assets/asset_metadata.hpp"
#include "assets/mesh_importer.hpp"
#include "assets/mesh_exporter.hpp"

#include <mill/mill.hpp>
#include <imgui.h>
#include <portable-file-dialogs.h>

#include <string>
#include <format>
#include <filesystem>

auto get_converted_mem_str(mill::u64 in_bytes) -> std::string
{
    const mill::f32 kilobyte_coefficient = 1024.0f;
    const mill::f32 megabyte_coefficient = kilobyte_coefficient * 1024.0f;
    const mill::f32 gigabyte_coefficient = megabyte_coefficient * 1024.0f;

    auto memory = static_cast<mill::f32>(in_bytes);

    if (memory < kilobyte_coefficient)
    {
        return std::format("{}{}", memory, "B");
    }
    else if (memory < megabyte_coefficient)
    {
        return std::format("{}{}", memory / kilobyte_coefficient, "KB");
    }
    else if (memory < gigabyte_coefficient)
    {
        return std::format("{}{}", memory / megabyte_coefficient, "MB");
    }

    return std::format("{}{}", memory / gigabyte_coefficient, "GB");
}

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

        platform::WindowInfo window_info{
            .title = "Asset Browser",
            .width = 1600,
            .height = 900,
        };
        m_windowHandle = platform::create_window(window_info);

        rhi::assign_screen(g_PrimaryScreenId, platform::get_raw_window_handle(m_windowHandle));
        rhi::reset_screen(g_PrimaryScreenId, 1600, 900, true);

        init_imgui();

        m_renderer = CreateOwned<Renderer>(g_MainViewId);
        m_renderer->initialise();

        m_assetBrowserView.inititialise(m_assetRegistry);

        // Load and export all assets in asset directory
        for (auto& dir_entry : std::filesystem::recursive_directory_iterator(g_AssetPath))
        {
            if (!dir_entry.is_regular_file())
            {
                continue;
            }

            const auto filePath = dir_entry.path();
            const auto ext = filePath.extension().string();

            const auto asset_type = get_asset_type(ext);
            if (asset_type == AssetType::eNone)
            {
                continue;
            }

            LOG_INFO("AssetBrowser - Importing asset <{}>", filePath.string());

            switch (asset_type)
            {
                case AssetType::eModel: handle_static_mesh(filePath.string()); break;
                default: break;
            }
        }
    }

    void AssetBrowserApp::shutdown()
    {
        shutdown_imgui();

        m_renderer->shutdown();
        m_renderer = nullptr;

        platform::destroy_window(m_windowHandle);
    }

    void AssetBrowserApp::update(f32 delta_time)
    {
        auto& io = ImGui::GetIO();
        io.DeltaTime = std::max(0.01f, delta_time);

        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open Project..."))
                {
                    open_project();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit"))
                {
                    Engine::get()->quit();
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Project"))
            {
                if (ImGui::MenuItem("Import..."))
                {
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Bake & Export"))
                {
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Asset"))
            {
                ImGui::TextDisabled("Selected Assets");
                if (ImGui::MenuItem("Delete"))
                {
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspace_flags);

        // #TODO: Application update
        // #TODO: ImGui Render

        if (ImGui::Begin("Stats"))
        {
            auto mem_stats = rhi::get_memory_stats();

            ImGui::Text("Device Memory: %s/%s",
                        get_converted_mem_str(mem_stats.DeviceTotalUsage).c_str(),
                        get_converted_mem_str(mem_stats.DeviceTotalBudget).c_str());
        }
        ImGui::End();

        static bool s_ShowDemo = true;
        ImGui::ShowDemoWindow(&s_ShowDemo);

        m_assetBrowserView.render();

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

    void AssetBrowserApp::reload_project()
    {
        LOG_INFO("AssetBrowser - Reloading project.");

        m_assetRegistry.clear();

        scan_for_and_register_assets(m_projectDir / "assets");

        m_assetBrowserView.refresh();
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

            auto& io = ImGui::GetIO();
            io.DisplaySize.x = CAST_F32(event.data.u32[0]);
            io.DisplaySize.y = CAST_F32(event.data.u32[1]);
        }
        else if (event.type == EventType::eInputMouseMove)
        {
            auto& io = ImGui::GetIO();
            io.AddMousePosEvent(CAST_F32(event.data.u32[0]), CAST_F32(event.data.u32[1]));

            if (!io.WantCaptureMouse)
            {
                // #TODO: Forward input to Game
            }
        }
        else if (event.type == EventType::eInputKey)
        {
            auto& io = ImGui::GetIO();
            // io.AddKeyEvent(event.data.u32[0], event.data.u32[1]);

            if (!io.WantCaptureKeyboard)
            {
                // #TODO: Forward input to Game
            }
        }
        else if (event.type == EventType::eInputMouseBtn)
        {
            auto& io = ImGui::GetIO();
            io.AddMouseButtonEvent(event.data.u32[0], event.data.u32[1]);

            if (!io.WantCaptureMouse)
            {
                // #TODO: Forward input to Game
            }
        }
    }

    void AssetBrowserApp::init_imgui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        io.Fonts->AddFontDefault();
        // io.Fonts->AddFontFromFileTTF("", 18.0f, nullptr, io.Fonts->GetGlyphRangesDefault());

        io.DisplaySize.x = 1600;
        io.DisplaySize.y = 900;
    }

    void AssetBrowserApp::shutdown_imgui()
    {
        ImGui::DestroyContext();
    }

    void AssetBrowserApp::open_project()
    {
        const std::string dialog_title = "Select project root directory...";
        const std::string dialog_default_path{};
        const auto dialog_options = pfd::opt::none;
        auto selected_dir = pfd::select_folder(dialog_title, dialog_default_path, dialog_options).result();

        open_project(selected_dir);
    }

    void AssetBrowserApp::open_project(const std::filesystem::path& project_dir)
    {
        const auto data_dir = project_dir / "data";
        const auto assets_dir = project_dir / "assets";
        if (!std::filesystem::exists(data_dir) || !std::filesystem::exists(assets_dir))
        {
            LOG_ERROR("AssetBrowser - Tried opening directory as project, but it didn't have either a assets or data sub-directory: {}",
                      project_dir.string());
            return;
        }

        LOG_INFO("AssetBrowser - Opening project dir: {}.", project_dir.string());

        m_projectDir = project_dir;

        m_assetBrowserView.set_root_dir(assets_dir);

        reload_project();
    }

    void AssetBrowserApp::scan_for_and_register_assets(const fs::path& dir_to_scan)
    {
        LOG_INFO("AssetBrowser - Scanning for assets in directory <{}>.", dir_to_scan.string());

        for (const auto& dir_entry : fs::recursive_directory_iterator(dir_to_scan))
        {
            if (!dir_entry.is_regular_file())
                continue;

            const auto& file_path = dir_entry.path();
            const auto& file_name = file_path.filename();

            const bool is_metadata_file = file_name.string().ends_with(".meta");
            if (!is_metadata_file)
                continue;

            LOG_DEBUG("AssetBrowser - Found asset metadata: {}.", file_path.string());

            const auto metadata = AssetMetadata::from_file(file_path);
            m_assetRegistry.register_metadata(metadata);
        }
    }

}
