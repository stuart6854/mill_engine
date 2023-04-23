#pragma once

#include "asset_metadata.hpp"

#include <mill/mill.hpp>

#include <unordered_map>

namespace mill::asset_browser
{
    class AssetRegistry
    {
    public:
        void clear();

        void register_metadata(const AssetMetadata& metadata);
        void unregister_metadata(u64 asset_id);

        auto get_metadata(u64 asset_id) const -> const AssetMetadata&;
        auto get_metadata_ref(u64 asset_id) -> AssetMetadata&;

        auto get_asset_id(const std::filesystem::path asset_path) const -> u64;

    private:
        std::unordered_map<u64, AssetMetadata> m_metadataMap{};
        std::unordered_map<std::filesystem::path, u64> m_assetIdMap{};
    };

}