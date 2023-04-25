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

        virtual void import_asset(const fs::path& asset_filename) = 0;

        virtual void render();

        /* Getters */

        auto get_name() const -> const std::string&;
        auto get_resource_id() const -> u64;
        auto get_resource_flags() const -> ResourceFlags;
        auto get_resource() -> const Shared<Resource>&;

        /* Operators */

        auto operator=(const ExportSettings&) -> ExportSettings& = delete;
        auto operator=(ExportSettings&&) -> ExportSettings& = default;

    protected:
        void set_resource(const Shared<Resource>& resource);

    private:
        std::string m_name{};
        u64 m_resourceId{};  // The exported resource id
        ResourceFlags m_resourceFlags{};

        Shared<Resource> m_resource{ nullptr };
    };

    auto create_asset_settings(AssetType type) -> Shared<ExportSettings>;
}