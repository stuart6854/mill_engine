#include "asset_export_settings.hpp"

#include "asset_metadata.hpp"
#include "export_settings/export_settings_model.hpp"

#include <mill/mill.hpp>

#include <imgui.h>

namespace mill::asset_browser
{
    ExportSettings::ExportSettings() : m_name("unamed_settings"), m_resourceId(random::random_u64()) {}

    void ExportSettings::write(YAML::Emitter& out)
    {
        out << YAML::Key << "name" << YAML::Key << m_name;
        out << YAML::Key << "resource_id" << YAML::Key << m_resourceId;
    }

    void ExportSettings::read(const YAML::Node& settings_root_node)
    {
        if (settings_root_node["name"])
            m_name = settings_root_node["name"].as<std::string>();
        if (settings_root_node["resource_id"])
            m_resourceId = settings_root_node["resource_id"].as<u64>();
    }

    void ExportSettings::render()
    {
        ImGui::Text("Resource Id: %i", m_resourceId);
    }

    auto ExportSettings::get_name() const -> const std::string&
    {
        return m_name;
    }

    auto ExportSettings::get_resource_id() const -> u64
    {
        return m_resourceId;
    }

    auto ExportSettings::get_resource_flags() const -> ResourceFlags
    {
        return m_resourceFlags;
    }

    auto create_asset_settings(AssetType type) -> Shared<ExportSettings>
    {
        switch (type)
        {
            case mill::asset_browser::AssetType::eModel: return std::move(CreateOwned<ExportSettingsModel>());
            default: ASSERT(("Unknown AssetType!", false)); break;
        }
        return nullptr;
    }

}
