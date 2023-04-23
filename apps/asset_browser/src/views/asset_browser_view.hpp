#pragma once

#include <mill/mill.hpp>

#include <filesystem>

namespace mill::asset_browser
{
    struct AssetMetadata;
    class AssetRegistry;

    class AssetBrowserView
    {
    public:
        void inititialise(AssetRegistry& asset_registry);

        void set_root_dir(const std::filesystem::path& root_dir);

        /* Clear and rebuild file tree structure. */
        void refresh();

        void render();

    private:
        struct TreeNode;
        void recurse_dir_tree(TreeNode& parent_node, const std::filesystem::path& current_dir);
        auto add_dir_tree_node(TreeNode& parent_node, const std::filesystem::path& path, bool is_folder) -> TreeNode&;

        void render_dir_tree_node(TreeNode& node);

    private:
        AssetRegistry* m_assetRegistry;
        std::filesystem::path m_rootDir{};

        struct TreeNode
        {
            u64 asset_id{};
            const AssetMetadata* metadata{ nullptr };

            std::filesystem::path path{};
            std::string name{};

            bool is_folder{};
            std::vector<TreeNode> children{};
        };

        Owned<TreeNode> m_rootNode{};

        bool m_showUnimportedAssets{ true };
    };
}