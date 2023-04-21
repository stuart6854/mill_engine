#include "asset_metadata.hpp"

#include <string>
#include <string_view>
#include <unordered_map>

namespace mill::asset_browser
{
    auto get_asset_type(const std::string& ext) -> AssetType
    {
        static const std::unordered_map<std::string, AssetType> s_AssetTypeMap{
            { ".obj", AssetType::eModel },
            { ".fbx", AssetType::eModel },
            { ".gltf", AssetType::eModel },
        };

        const auto it = s_AssetTypeMap.find(ext);
        if (it != s_AssetTypeMap.end())
            return it->second;

        return AssetType::eNone;
    }

}
