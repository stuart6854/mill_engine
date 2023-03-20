#pragma once

#include <mill/mill.hpp>

#include <string>

namespace mill::asset_browser
{
    void export_static_mesh(StaticMesh& mesh, const std::string& filename);
}