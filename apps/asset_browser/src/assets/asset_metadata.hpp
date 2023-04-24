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
        eTexture2D,
    };

    struct AssetMetadata
    {
        u64 id{};
        std::string name{};
        std::filesystem::path assetFilename{};
        AssetType type{};
        u64 fileSizeBytes{};

        static void to_file(const AssetMetadata& metadata, const std::filesystem::path& filename);
        static auto from_file(const std::filesystem::path& filename) -> AssetMetadata;
    };

    auto get_asset_type(const std::string& ext) -> AssetType;
    auto get_asset_type_str(AssetType type) -> std::string;
}