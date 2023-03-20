#pragma once

#include "mill/core/base.hpp"
#include "resource.hpp"

#include <unordered_map>

namespace mill
{
    struct ResourceFactorySystems;

    class ResourceCache
    {
    public:
        explicit ResourceCache() = default;
        ~ResourceCache() = default;

        DISABLE_COPY_AND_MOVE(ResourceCache);

        void add(ResourceId id, Owned<Resource> resource);
        auto get(ResourceId id) -> Resource*;
        void erase(ResourceId id);
        bool contains(ResourceId id);

    private:
        std::unordered_map<ResourceId, Owned<Resource>> m_cache{};
    };

}