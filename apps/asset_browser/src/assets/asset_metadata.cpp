#include "asset_metadata.hpp"

#include <yaml-cpp/yaml.h>

#include <string>
#include <string_view>
#include <unordered_map>
#include <fstream>

namespace mill::asset_browser
{

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

        // #TODO: Read settings based on `AssetType`

        return metadata;
    }

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
