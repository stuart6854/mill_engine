#include "asset_browser_view.hpp"

#include <imgui.h>

#include <vector>
#include <string>

namespace mill::asset_browser
{
    void AssetBrowserView::inititialise() {}

    void AssetBrowserView::render()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
        if (ImGui::Begin("Asset Browser"))
        {
            static ImGuiTableFlags table_flags = ImGuiTableFlags_BordersV;

            if (ImGui::BeginTable("asset_browser_table", 4, table_flags, ImVec2(-1, -1)))
            {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("Flags");
                ImGui::TableHeadersRow();

                // File
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TreeNodeEx("asset_0",
                                  ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
                ImGui::TableNextColumn();
                ImGui::Text("StaticMesh");
                ImGui::TableNextColumn();
                ImGui::Text("%s", "125MB");
                ImGui::TableNextColumn();

                // Folder
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                bool open = ImGui::TreeNodeEx("some_folder", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth);
                ImGui::TableNextColumn();
                ImGui::TextDisabled("--");
                ImGui::TableNextColumn();
                ImGui::Text("%s", "110KB");
                ImGui::TableNextColumn();
                ImGui::TextDisabled("--");
                ImGui::TableNextColumn();
                if (open)
                {
                    // Child File
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TreeNodeEx("asset_1",
                                      ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
                    ImGui::TableNextColumn();
                    ImGui::Text("Texture2D");
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", "55KB");
                    ImGui::TableNextColumn();

                    // Child File
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TreeNodeEx("asset_2",
                                      ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
                    ImGui::TableNextColumn();
                    ImGui::Text("SkeletalMesh");
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", "55KB");
                    ImGui::TableNextColumn();

                    ImGui::TreePop();
                }

                ImGui::EndTable();
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

}
