#pragma once

#include <mill/mill.hpp>

namespace mill::asset_browser
{
    class AssetPreviewView
    {
    public:
        void init(u64 scene_view_id);

        void render();

    private:
        u64 m_sceneViewId{};
        u64 m_resourceSetId{};

        Entity m_entity{};
    };
}