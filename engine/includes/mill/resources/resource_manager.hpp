#pragma once

#include "mill/core/base.hpp"
#include "resource.hpp"
#include "resource_cache.hpp"
#include "resource_factory.hpp"

#include <set>
#include <queue>
#include <filesystem>
namespace fs = std::filesystem;
#include <unordered_map>

namespace mill
{
    class ResourceManager;

    class ResourceHandle
    {
    public:
        explicit ResourceHandle() = default;
        explicit ResourceHandle(ResourceManager& manager, ResourceId id);
        explicit ResourceHandle(Resource* resource);
        ResourceHandle(const ResourceHandle& other);
        ResourceHandle(ResourceHandle&& other) noexcept;
        ~ResourceHandle() = default;

        /* Getters */

        template <typename T>
        auto As() -> T*;

        /* Operators */

        auto operator=(const ResourceHandle& rhs) -> ResourceHandle&;
        auto operator=(ResourceHandle&& rhs) noexcept -> ResourceHandle&;

        operator bool() const;

    private:
        ResourceManager* m_manager{ nullptr };
        ResourceId m_id{};
        ResourceMetadata* m_metadata{ nullptr };
        Resource* m_resource{ nullptr };
        RefCount m_refCount{};
    };

    struct ResourceManagerInit
    {
        std::string resourcePath = "..\\..\\data";
    };

    class ResourceManager
    {
    public:
        explicit ResourceManager() = default;
        ~ResourceManager() = default;

        DISABLE_COPY_AND_MOVE(ResourceManager);

        /* Commands */

        void initialise(const ResourceManagerInit& init);
        void shutdown();

        template <typename ResourceType>
        void register_resource_type(ResourceTypeId resource_type_id, Owned<ResourceFactory> factory);
        // void register_factory(ResourceType resource_type, );

        /* Getters */

        auto get_metadata(ResourceId id) -> ResourceMetadata&;

        auto get_handle(ResourceId id) -> ResourceHandle;
        auto get_resource(ResourceId id) -> Resource*;

    private:
        void load_all_metadata();
        void load_metadata_file(const fs::path& filename);

        /* Adds resource to queue to be loaded by worker thread. */
        void load_resource(ResourceId id);

        /* Loads immediately on current thread. */
        void force_load_resource();

    private:
        fs::path m_resourcePath;

        std::unordered_map<ResourceId, ResourceMetadata> m_metadataMap{};
        std::unordered_map<ResourceTypeId, Owned<ResourceCacheBase>> m_resourceCaches{};
        std::unordered_map<ResourceTypeId, Owned<ResourceFactory>> m_resourceFactories{};

        std::set<ResourceId> m_pendingResources{};
        std::queue<ResourceId> m_resourceLoadQueue{};
    };

    template <typename ResourceType>
    void mill::ResourceManager::register_resource_type(ResourceTypeId resource_type_id, Owned<ResourceFactory> factory)
    {
        m_resourceCaches[resource_type_id] = CreateOwned<ResourceCache<ResourceType>>();
        m_resourceFactories[resource_type_id] = std::move(factory);
    }

    template <typename T>
    inline auto ResourceHandle::As() -> T*
    {
        Resource* resource{ nullptr };
        if (m_manager && m_id)
        {
            resource = m_manager->get_resource(m_id);
        }
        else if (m_resource != nullptr)
        {
            resource = m_resource;
        }

        const auto* cast_resource = static_cast<T*>(resource);
        return cast_resource;
    }
}