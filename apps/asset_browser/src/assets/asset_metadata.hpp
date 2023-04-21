#pragma once

#include <mill/mill.hpp>

#include <string>
#include <string_view>
#include <filesystem>

namespace mill::asset_browser
{
    enum class AssetType
    {
        eNone,
        eModel,
    };

    struct AssetMetadata
    {
        std::string name{};
        std::filesystem::path assetFilename{};
        AssetType type{};
        u64 fileSizeBytes{};
    };

    auto get_asset_type(const std::string& ext) -> AssetType;
}