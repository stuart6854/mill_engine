#include "asset_browser_view.hpp"

#include "../assets/asset_metadata.hpp"
#include "../assets/asset_registry.hpp"

#include <imgui.h>

#include <vector>
#include <string>

namespace mill::asset_browser
{
    void AssetBrowserView::inititialise(AssetRegistry& asset_registry)
    {
        m_assetRegistry = &asset_registry;
        m_rootNode = CreateOwned<TreeNode>();
    }

    void AssetBrowserView::set_root_dir(const std::filesystem::path& root_dir)
    {
        m_rootDir = root_dir;
    }

    void AssetBrowserView::refresh()
    {
        *m_rootNode = {};
        recurse_dir_tree(*m_rootNode, m_rootDir);
    }

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

#if 0
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
#endif

                for (auto& child_node : m_rootNode->children)
                    render_dir_tree_node(child_node);

                ImGui::EndTable();
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void AssetBrowserView::recurse_dir_tree(TreeNode& current_node, const std::filesystem::path& current_dir)
    {
        for (const auto& dir_entry : std::filesystem::directory_iterator(current_dir))
        {
            if (!dir_entry.is_regular_file() && !dir_entry.is_directory())
                continue;

            if (dir_entry.is_symlink())
                continue;

            if (dir_entry.is_directory())
            {
                const auto& dir_path = dir_entry.path();
                auto& node = add_dir_tree_node(current_node, dir_path, true);
                recurse_dir_tree(node, dir_path);
            }
            else if (dir_entry.is_regular_file())
            {
                const auto& file_path = dir_entry.path();
                const auto& file_ext = dir_entry.path().extension();
                if (file_ext != ".meta")
                {
                    // Is this file a potential importable asset?
                    const auto asset_type = get_asset_type(file_ext.string());
                    const bool is_asset_type = asset_type != AssetType::eNone;
                    if (is_asset_type)
                    {
                        add_dir_tree_node(current_node, file_path, false);
                    }
                }
            }
        }
    }

    auto AssetBrowserView::add_dir_tree_node(TreeNode& parent_node, const std::filesystem::path& path, bool is_folder) -> TreeNode&
    {
        auto& node = parent_node.children.emplace_back();
        node.path = path;
        node.name = path.filename().string();
        node.is_folder = is_folder;
        if (!is_folder)
        {
            node.asset_id = m_assetRegistry->get_asset_id(path);
            if (node.asset_id != 0)
            {
                node.metadata = &m_assetRegistry->get_metadata(node.asset_id);
            }
        }

        return node;
    }

    void AssetBrowserView::render_dir_tree_node(TreeNode& node)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (node.is_folder)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            bool open = ImGui::TreeNodeEx(node.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth);
            ImGui::TableNextColumn();
            ImGui::TextDisabled("--");
            ImGui::TableNextColumn();
            ImGui::Text("%s", "<dir_size>");
            ImGui::TableNextColumn();
            ImGui::TextDisabled("--");
            ImGui::TableNextColumn();
            if (open)
            {
                for (auto& child_node : node.children)
                {
                    render_dir_tree_node(child_node);
                }
                ImGui::TreePop();
            }
        }
        else
        {
            const bool is_imported_asset = node.asset_id > 0;

            const auto name = is_imported_asset ? node.name : std::format("{} (Not imported)", node.name);
            ImGui::TreeNodeEx(name.c_str(),
                              ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
            ImGui::TableNextColumn();

            ImGui::Text(is_imported_asset ? get_asset_type_str(node.metadata->type).c_str() : "---");
            ImGui::TableNextColumn();
            ImGui::Text("%s", "<asset_size>");
            ImGui::TableNextColumn();
        }
    }

}
