#pragma once

#include "mill/resources/resource.hpp"

namespace mill
{
    struct ResourceMetadata;

    struct ResourceFactory
    {
        virtual auto load(const ResourceMetadata& metadata) -> Owned<Resource> = 0;
    };

    static const std::string g_StaticMeshHeader = "msm";
    struct StaticMeshFactory : public ResourceFactory
    {
        auto load(const ResourceMetadata& metadata) -> Owned<Resource> override;
    };
}