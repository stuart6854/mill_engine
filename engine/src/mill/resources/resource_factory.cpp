#include "mill/resources/resource_factory.hpp"

namespace mill
{
    auto StaticMeshFactory::load(const ResourceMetadata& metadata) -> Owned<Resource>
    {
        // #TODO: Load resource from disk
        UNUSED(metadata);

        auto static_mesh = m_renderer->create_static_mesh();
        // #TODO: Set resource data
        static_mesh->set_vertices({});
        static_mesh->set_triangles({});
        static_mesh->apply();
        return std::move(static_mesh);
    }

}
