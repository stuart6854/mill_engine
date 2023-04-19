#pragma once

#include "mill/resources/resource_manager.hpp"

namespace mill
{
    struct StaticMeshComponent
    {
        StaticMeshComponent() = default;
        StaticMeshComponent(const StaticMeshComponent&) = default;

        ResourceHandle staticMesh{};
    };
}