#include "asset_registry.hpp"

namespace mill::asset_browser
{
    void AssetRegistry::clear()
    {
        m_metadataMap.clear();
    }

    void AssetRegistry::register_metadata(const AssetMetadata& metadata)
    {
        ASSERT(metadata.id);
        ASSERT(("Asset already registered!", !m_metadataMap.contains(metadata.id)));

        m_metadataMap[metadata.id] = metadata;
        m_assetIdMap[metadata.assetFilename] = metadata.id;

        LOG_DEBUG("AssetBrowser - AssetRegistry - Asset metadata registered: id={}, name={}, type={}",
                  metadata.id,
                  metadata.name,
                  get_asset_type_str(metadata.type));
    }

    void AssetRegistry::unregister_metadata(u64 asset_id)
    {
        if (!m_metadataMap.contains(asset_id))
            return;

        const auto& metadata = get_metadata(asset_id);
        m_assetIdMap.erase(metadata.assetFilename);
        m_metadataMap.erase(asset_id);

        LOG_DEBUG("AssetBrowser - AssetRegistry - Asset metadata unregistered: id={}", asset_id);
    }

    auto AssetRegistry::get_metadata(u64 asset_id) const -> const AssetMetadata&
    {
        ASSERT(asset_id);
        ASSERT(("Asset does not exist!", m_metadataMap.contains(asset_id)));

        return m_metadataMap.at(asset_id);
    }

    auto AssetRegistry::get_metadata_ref(u64 asset_id) -> AssetMetadata&
    {
        ASSERT(asset_id);
        ASSERT(("Asset does not exist!", m_metadataMap.contains(asset_id)));

        return m_metadataMap.at(asset_id);
    }

    auto AssetRegistry::get_asset_id(const std::filesystem::path asset_path) const -> u64
    {
        const auto it = m_assetIdMap.find(asset_path);
        if (it == m_assetIdMap.end())
            return 0;

        return it->second;
    }

}
