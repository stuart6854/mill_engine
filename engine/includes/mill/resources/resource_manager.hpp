#pragma once

#include "mill/core/base.hpp"
#include "resource.hpp"

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

        /* Getters */

        auto get_metadata(ResourceId id) -> ResourceMetadata&;

        auto get_handle(ResourceId id) -> ResourceHandle;
        auto get_resource(ResourceId id) -> Resource*;

    private:
        void load_all_metadata();
        void load_metadata_file(const fs::path& filename);

        void load_resource(ResourceId id);

    private:
        fs::path m_resourcePath;

        std::unordered_map<ResourceId, ResourceMetadata> m_metadataMap{};
        std::unordered_map<ResourceId, Owned<Resource>> m_resourceMap{};

        std::set<ResourceId> m_pendingResources{};
        std::queue<ResourceId> m_resourceLoadQueue{};
    };

    template <typename T>
    inline auto ResourceHandle::As() -> T*
    {
        Resource* resource{ nullptr };
        const auto* cast_resource = static_cast<T*>(resource);
        return cast_resource;
    }
}