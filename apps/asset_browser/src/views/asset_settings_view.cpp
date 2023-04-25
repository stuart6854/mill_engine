#include "asset_settings_view.hpp"

#include "../assets/asset_metadata.hpp"
#include "../assets/asset_export_settings.hpp"

#include <imgui.h>

#include <format>

namespace mill::asset_browser
{
    void AssetSettingView::set_active_asset(AssetMetadata* asset_metadata)
    {
        m_activeMetadata = asset_metadata;
    }

    void AssetSettingView::render()
    {
        if (ImGui::Begin("Asset Settings"))
        {
            ImGui::Text("Settings");

            if (m_activeMetadata != nullptr)
            {
                if (ImGui::Button("Add Export Settings"))
                {
                    auto new_settings = create_asset_settings(m_activeMetadata->type);
                    m_activeMetadata->exportSettings.push_back(new_settings);
                }

                ImGui::SameLine();

                if (ImGui::Button("Apply Settings"))
                {
                    auto metadata_filename = fs::path(m_activeMetadata->assetFilename).concat(".meta");
                    AssetMetadata::to_file(*m_activeMetadata, metadata_filename);

                    m_activeMetadata->import_asset();
                }

                const auto& export_settings = m_activeMetadata->exportSettings;
                for (auto i = 0; i < export_settings.size(); ++i)
                {
                    const auto& settings = export_settings[i];
                    static const auto collapse_header_flags = ImGuiTreeNodeFlags_None;

                    auto label = std::format("{}###{}", settings->get_name(), i);
                    if (ImGui::CollapsingHeader(label.c_str(), collapse_header_flags))
                    {
                        settings->render();
                    }
                }
            }
        }
        ImGui::End();
    }

}
