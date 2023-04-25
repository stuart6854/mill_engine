#pragma once

#include "asset_type.hpp"
#include "asset_export_settings.hpp"

#include <mill/mill.hpp>

#include <vector>
#include <string>
#include <filesystem>

namespace mill::asset_browser
{
    struct AssetMetadata
    {
        u64 id{};
        std::string name{};
        std::filesystem::path assetFilename{};
        AssetType type{};
        u64 fileSizeBytes{};

        std::vector<Shared<ExportSettings>> exportSettings{};

        void import_asset();

        static void to_file(const AssetMetadata& metadata, const std::filesystem::path& filename);
        static auto from_file(const std::filesystem::path& filename) -> AssetMetadata;
    };
}