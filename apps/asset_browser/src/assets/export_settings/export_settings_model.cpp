#include "export_settings_model.hpp"

#include <mill/mill.hpp>

#include <imgui.h>

namespace mill::asset_browser
{
    namespace
    {
        auto to_string(MeshType type) -> std::string
        {
            switch (type)
            {
                case mill::asset_browser::MeshType::eStatic: return "Static";
                case mill::asset_browser::MeshType::eSkeletal: return "Skeletal";
                default: ASSERT(("Unknown MeshType!", false)); break;
            }
            return "<unknown>";
        }
    }

    void ExportSettingsModel::write(YAML::Emitter& out)
    {
        ExportSettings::write(out);

        out << YAML::Key << "mesh_type" << YAML::Key << static_cast<i32>(m_type);
        out << YAML::Key << "lod_count" << YAML::Key << m_lodCount;
    }

    void ExportSettingsModel::read(const YAML::Node& settings_root_node)
    {
        ExportSettings::read(settings_root_node);

        if (settings_root_node["mesh_type"])
            m_type = static_cast<MeshType>(settings_root_node["mesh_type"].as<i32>());
        if (settings_root_node["lod_count"])
            m_lodCount = settings_root_node["lod_count"].as<u32>();
    }

    void ExportSettingsModel::render()
    {
        ExportSettings::render();

        ImGui::Combo("Mesh Type", reinterpret_cast<i32*>(&m_type), "Static\0Skeletal\0\0");
        ImGui::DragInt("Lod Count", reinterpret_cast<i32*>(&m_lodCount), 1.0f, 1, 10);
    }

}
