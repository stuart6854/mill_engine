#pragma once

namespace mill::asset_browser
{
    struct AssetMetadata;

    class AssetSettingView
    {
    public:
        void set_active_asset(AssetMetadata* asset_metadata);

        void render();

    private:
        AssetMetadata* m_activeMetadata;
    };
}