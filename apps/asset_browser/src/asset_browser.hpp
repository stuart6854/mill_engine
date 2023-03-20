#pragma once

#include <mill/mill.hpp>

namespace mill::asset_browser
{
    class AssetBrowserApp : public mill::Application
    {
    public:
        void initialise() override;
        void shutdown() override;
    };
}