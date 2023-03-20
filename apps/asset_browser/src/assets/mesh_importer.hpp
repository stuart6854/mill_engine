#pragma once

#include <mill/mill.hpp>

#include <string>

namespace mill::asset_browser
{
    auto import_static_mesh(const std::string& filename) -> Owned<StaticMesh>;
}