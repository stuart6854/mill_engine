#include "asset_settings_view.hpp"

#include "../assets/asset_metadata.hpp"
#include "../assets/asset_export_settings.hpp"

#include <imgui.h>

namespace mill::asset_browser
{
    void AssetSettingView::render()
    {
        if (ImGui::Begin("Asset Settings"))
        {
            ImGui::Text("Settings");

            if (m_activeMetadata != nullptr)
            {
                for (const auto& settings : m_activeMetadata->exportSettings)
                {
                    UNUSED(settings);
                    static const auto collapse_header_flags = ImGuiTreeNodeFlags_None;
                    // #TODO: Settings name
                    if (ImGui::CollapsingHeader("<settings_name>", collapse_header_flags))
                    {
                    }
                }
            }
        }
        ImGui::End();
    }

}
