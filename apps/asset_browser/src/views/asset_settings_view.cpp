#include "asset_settings_view.hpp"

#include "../assets/asset_metadata.hpp"
#include "../assets/asset_export_settings.hpp"

#include <imgui.h>

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

                for (const auto& settings : m_activeMetadata->exportSettings)
                {
                    static const auto collapse_header_flags = ImGuiTreeNodeFlags_None;
                    if (ImGui::CollapsingHeader("<settings_name>", collapse_header_flags))
                    {
                        settings->render();
                    }
                }
            }
        }
        ImGui::End();
    }

}
