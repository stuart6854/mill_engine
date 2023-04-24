#include "asset_export_settings.hpp"

#include "asset_metadata.hpp"
#include "export_settings/export_settings_model.hpp"

#include <mill/mill.hpp>

namespace mill::asset_browser
{
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
