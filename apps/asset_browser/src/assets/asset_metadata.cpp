#include "asset_metadata.hpp"

#include "asset_type.hpp"
#include "asset_export_settings.hpp"

#include <yaml-cpp/yaml.h>

#include <string>
#include <string_view>
#include <unordered_map>
#include <fstream>

namespace mill::asset_browser
{
    void AssetMetadata::import_asset()
    {
        for (auto& settings : exportSettings)
            settings->import_asset(assetFilename);
    }

    void AssetMetadata::to_file(const AssetMetadata& metadata, const std::filesystem::path& filename)
    {
        YAML::Emitter out{};

        out << YAML::BeginMap;
        out << YAML::Key << "metadata";
        {
            out << YAML::BeginMap;
            out << YAML::Key << "id" << YAML::Value << metadata.id;
            out << YAML::Key << "name" << YAML::Value << metadata.name;
            out << YAML::Key << "type" << YAML::Value << enum_to_underlying(metadata.type);
            out << YAML::Key << "settings";
            out << YAML::BeginSeq;
            {
                for (const auto& settings : metadata.exportSettings)
                {
                    out << YAML::BeginMap;
                    {
                        settings->write(out);
                    }
                    out << YAML::EndMap;
                }
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;
        }
        out << YAML::EndMap;

        std::ofstream file(filename, std::ios::trunc);
        file << out.c_str();
        file.close();
    }

    auto AssetMetadata::from_file(const std::filesystem::path& filename) -> AssetMetadata
    {
        ASSERT(std::filesystem::exists(filename));

        AssetMetadata metadata{};

        auto root_node = YAML::LoadFile(filename.string());
        if (!root_node["metadata"])
            return {};

        const auto& metadata_node = root_node["metadata"];

        metadata.id = metadata_node["id"].as<u64>();
        metadata.name = metadata_node["name"].as<std::string>();
        metadata.assetFilename = std::filesystem::path(filename).replace_extension();
        metadata.type = static_cast<AssetType>(metadata_node["type"].as<u32>());

        const auto& settings_node = metadata_node["settings"];
        if (settings_node)
        {
            for (auto i = 0; i < settings_node.size(); ++i)
            {
                auto settings = create_asset_settings(metadata.type);
                metadata.exportSettings.push_back(settings);

                settings->read(settings_node[i]);
            }
        }

        metadata.import_asset();

        return metadata;
    }

}
