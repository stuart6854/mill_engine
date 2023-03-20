#include "mill/resources/resource_cache.hpp"

namespace mill
{
    void ResourceCache::add(ResourceId id, Owned<Resource> resource)
    {
        m_cache[id] = std::move(resource);
    }

    auto ResourceCache::get(ResourceId id) -> Resource*
    {
        return m_cache.at(id).get();
    }

    void mill::ResourceCache::erase(ResourceId id)
    {
        m_cache.erase(id);
    }

    bool mill::ResourceCache::contains(ResourceId id)
    {
        return m_cache.contains(id);
    }

}
