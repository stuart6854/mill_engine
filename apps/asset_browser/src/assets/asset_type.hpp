#pragma once

#include <string>

namespace mill::asset_browser
{
    enum class AssetType
    {
        eNone,
        eModel,
        eTexture2D,
    };

    auto get_asset_type(const std::string& ext) -> AssetType;
    auto get_asset_type_str(AssetType type) -> std::string;
}