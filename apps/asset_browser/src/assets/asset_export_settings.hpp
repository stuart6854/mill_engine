#pragma once

#include "asset_type.hpp"

#include <mill/mill.hpp>
#include <yaml-cpp/yaml.h>

namespace mill::asset_browser
{
    class ExportSettings
    {
    public:
        ExportSettings();
        ExportSettings(const ExportSettings&) = delete;
        ExportSettings(ExportSettings&&) = default;

        virtual ~ExportSettings() = default;

        virtual void write(YAML::Emitter& out);
        virtual void read(const YAML::Node& settings_root_node);

        virtual void render();

        /* Getters */

        auto get_name() const -> const std::string&;
        auto get_resource_id() const -> u64;
        auto get_resource_flags() const -> ResourceFlags;

        /* Operators */

        auto operator=(const ExportSettings&) -> ExportSettings& = delete;
        auto operator=(ExportSettings&&) -> ExportSettings& = default;

    private:
        std::string m_name{};
        u64 m_resourceId{};  // The exported resource id
        ResourceFlags m_resourceFlags{};
    };

    auto create_asset_settings(AssetType type) -> Shared<ExportSettings>;
}