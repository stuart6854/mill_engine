#pragma once

#include "../asset_export_settings.hpp"

#include <mill/mill.hpp>

#include <yaml-cpp/yaml.h>

namespace mill::asset_browser
{
    enum class MeshType : u8
    {
        eStatic,
        eSkeletal,
    };

    class ExportSettingsModel : public ExportSettings
    {
    public:
        void write(YAML::Emitter& out) override;
        void read(const YAML::Node& settings_root_node) override;

        void render() override;

    private:
        MeshType m_type{};
        u32 m_lodCount{ 1 };
    };
}