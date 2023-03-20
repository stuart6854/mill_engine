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

    static const std::string g_StaticMeshHeader = "msm";
    struct StaticMeshFactory : public ResourceFactory
    {
        RendererInterface* m_renderer{ nullptr };

        StaticMeshFactory(RendererInterface* renderer) : m_renderer(renderer) {}

        auto load(const ResourceMetadata& metadata) -> Owned<Resource> override;
    };
}