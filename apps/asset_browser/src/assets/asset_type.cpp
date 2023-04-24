#include "asset_type.hpp"

#include <mill/mill.hpp>

#include <unordered_map>

namespace mill::asset_browser
{
    auto get_asset_type(const std::string& ext) -> AssetType
    {
        static const std::unordered_map<std::string, AssetType> s_AssetTypeMap{
            { ".obj", AssetType::eModel },     { ".fbx", AssetType::eModel },     { ".gltf", AssetType::eModel },
            { ".png", AssetType::eTexture2D }, { ".jpg", AssetType::eTexture2D },
        };

        const auto it = s_AssetTypeMap.find(ext);
        if (it != s_AssetTypeMap.end())
            return it->second;

        return AssetType::eNone;
    }

    auto get_asset_type_str(AssetType type) -> std::string
    {
        switch (type)
        {
            case mill::asset_browser::AssetType::eNone: return "None";
            case mill::asset_browser::AssetType::eModel: return "Model";
            case mill::asset_browser::AssetType::eTexture2D: return "Texture2D";
            default: ASSERT(("Unknown AssetType!", false)); break;
        }

        return "<unknown_type>";
    }
}
