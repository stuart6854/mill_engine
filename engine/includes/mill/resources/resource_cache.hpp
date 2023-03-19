#pragma once

#include "mill/core/base.hpp"
#include "resource.hpp"

#include <unordered_map>

namespace mill
{
    struct ResourceFactorySystems;

    class ResourceCacheBase
    {
    public:
        virtual ~ResourceCacheBase() = default;

        virtual auto get(ResourceId id) -> Resource* = 0;
        virtual void erase(ResourceId id) = 0;
        virtual bool contains(ResourceId id) = 0;
    };

    template <typename Resource>
    class ResourceCache : public ResourceCacheBase
    {
    public:
        explicit ResourceCache() = default;
        ~ResourceCache() = default;

        DISABLE_COPY_AND_MOVE(ResourceCache);

        auto get(ResourceId id) -> Resource* override;
        void erase(ResourceId id) override;
        bool contains(ResourceId id) override;

    private:
        std::unordered_map<ResourceId, Owned<Resource>> m_cache{};
    };

    template <typename Resource>
    auto ResourceCache<Resource>::get(ResourceId id) -> Resource*
    {
        return m_cache.at(id).get();
    }

    template <typename Resource>
    void mill::ResourceCache<Resource>::erase(ResourceId id)
    {
        m_cache.erase(id);
    }

    template <typename Resource>
    bool mill::ResourceCache<Resource>::contains(ResourceId id)
    {
        return m_cache.contains(id);
    }

}