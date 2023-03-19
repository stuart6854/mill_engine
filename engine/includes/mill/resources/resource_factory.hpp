#pragma once

#include "mill/resources/resource.hpp"
#include "mill/graphics/renderer.hpp"

namespace mill
{
    struct ResourceMetadata;

    struct ResourceFactory
    {
        virtual auto load(const ResourceMetadata& metadata) -> Owned<Resource> = 0;
    };

    struct StaticMeshFactory : public ResourceFactory
    {
        RendererInterface* m_renderer{ nullptr };

        auto load(const ResourceMetadata& metadata) -> Owned<Resource> override;
    };
}