#include <mill/entry_point.hpp>

#include "asset_browser.hpp"

auto mill::create_application() -> mill::Application*
{
    return new mill::asset_browser::AssetBrowserApp;
}