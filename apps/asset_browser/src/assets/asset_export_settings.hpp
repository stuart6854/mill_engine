#pragma once

#include "asset_type.hpp"

#include <mill/mill.hpp>
#include <yaml-cpp/yaml.h>

namespace mill::asset_browser
{
    class ExportSettings
    {
    public:
        ExportSettings() = default;
        ExportSettings(const ExportSettings&) = delete;
        ExportSettings(ExportSettings&&) = default;

        virtual ~ExportSettings() = default;

        virtual void write(YAML::Emitter& out) = 0;
        virtual void read(const YAML::Node& settings_root_node) = 0;

        virtual void render() = 0;

        /* Operators */

        auto operator=(const ExportSettings&) -> ExportSettings& = delete;
        auto operator=(ExportSettings&&) -> ExportSettings& = default;
    };

    auto create_asset_settings(AssetType type) -> Shared<ExportSettings>;
}