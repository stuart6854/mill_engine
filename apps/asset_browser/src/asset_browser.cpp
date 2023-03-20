#include "asset_browser.hpp"

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

    void AssetBrowserApp::shutdown() {}

}
