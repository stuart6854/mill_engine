#pragma once

#include <unordered_map>

namespace mill::asset_browser
{
    enum class AssetType
    {
        eNone,
        eMesh,
        eMaterial,
    };

    static const std::unordered_map<std::string, AssetType> g_AssetTypeMap{
        { ".obj", AssetType::eMesh },
        { ".fbx", AssetType::eMesh },
        { ".gltf", AssetType::eMesh },
    };

    static const auto* g_AssetPath = "..\\..\\assets";
}