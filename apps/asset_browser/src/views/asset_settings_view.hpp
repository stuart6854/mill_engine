#pragma once

namespace mill::asset_browser
{
    struct AssetMetadata;

    class AssetSettingView
    {
    public:
        void render();

    private:
        AssetMetadata* m_activeMetadata;
    };
}