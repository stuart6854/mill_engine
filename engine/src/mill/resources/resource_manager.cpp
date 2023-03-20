#include "mill/resources/resource_manager.hpp"

#include "mill/core/debug.hpp"

#include <filesystem>
namespace fs = std::filesystem;

#include <yaml-cpp/yaml.h>

namespace mill
{
    constexpr auto* g_MetadataFileExt = ".yaml";

    ResourceHandle::ResourceHandle(ResourceManager& manager, ResourceId id) : m_manager(&manager), m_id(id)
    {
        m_metadata = &m_manager->get_metadata(m_id);
        m_refCount = m_metadata->refCount;
    }

    ResourceHandle::ResourceHandle(Resource* resource) : m_resource(resource) {}

    ResourceHandle::ResourceHandle(const ResourceHandle& other)
    {
        m_manager = other.m_manager;
        m_id = other.m_id;
        m_metadata = other.m_metadata;
        m_refCount = other.m_refCount;
    }

    ResourceHandle::ResourceHandle(ResourceHandle&& other) noexcept
    {
        std::swap(m_manager, other.m_manager);
        std::swap(m_id, other.m_id);
        std::swap(m_metadata, other.m_metadata);
        std::swap(m_refCount, m_refCount);
    }

    auto ResourceHandle::operator=(const ResourceHandle& rhs) -> ResourceHandle&
    {
        m_manager = rhs.m_manager;
        m_id = rhs.m_id;
        m_metadata = rhs.m_metadata;
        m_refCount = rhs.m_refCount;
        return *this;
    }

    auto ResourceHandle::operator=(ResourceHandle&& rhs) noexcept -> ResourceHandle&
    {
        std::swap(m_manager, rhs.m_manager);
        std::swap(m_id, rhs.m_id);
        std::swap(m_metadata, rhs.m_metadata);
        std::swap(m_refCount, rhs.m_refCount);
        return *this;
    }

    ResourceHandle::operator bool() const
    {
        return m_metadata->isLoaded;
    }

    void ResourceManager::initialise(const ResourceManagerInit& init)
    {
        LOG_INFO("ResourceManager - Initialising...");
        m_resourcePath = init.resourcePath;

        load_all_metadata();
    }

    void ResourceManager::shutdown()
    {
        LOG_INFO("ResourceManager - Shutting down...");
        m_resourceLoadQueue = {};
        m_metadataMap.clear();
    }

    auto ResourceManager::get_metadata(ResourceId id) -> ResourceMetadata&
    {
        ASSERT(id);
        ASSERT(m_metadataMap.find(id) != m_metadataMap.end());

        return m_metadataMap[id];
    }

    auto ResourceManager::get_handle(ResourceId id, bool force_load) -> ResourceHandle
    {
        ASSERT(id);
        ASSERT(m_metadataMap.find(id) != m_metadataMap.end());

        const auto& metadata = get_metadata(id);
        if (!metadata.isLoaded)
        {
            if (!force_load)
            {
                load_resource(id);
            }
            else
            {
                force_load_resource(id);
            }
        }

        return ResourceHandle(*this, id);
    }

    auto ResourceManager::get_resource(ResourceId id) -> Resource*
    {
        ASSERT(id);
        ASSERT(m_metadataMap.contains(id));

        const auto& metadata = get_metadata(id);
        if (!metadata.isLoaded)
        {
            if (m_pendingResources.contains(id))
            {
                return nullptr;
            }

            force_load_resource(id);
            if (!metadata.isLoaded)
            {
                return nullptr;
            }
        }

        ASSERT(m_resourceCaches.contains(metadata.typeId));

        auto* cache = m_resourceCaches[metadata.typeId].get();
        return cache->get(id);
    }

    void ResourceManager::load_all_metadata()
    {
        LOG_INFO("ResourceManager - Loading all resource metadata.");

        // Assume .yaml files hold resource metadata
        for (const auto& dir_entry : fs::recursive_directory_iterator(m_resourcePath))
        {
            if (!dir_entry.is_regular_file())
            {
                continue;
            }

            const auto path = dir_entry.path();
            if (path.extension() != g_MetadataFileExt)
            {
                continue;
            }

            load_metadata_file(path);
        }
    }

    void ResourceManager::load_metadata_file(const fs::path& filename)
    {
        const YAML::Node rootNode = YAML::LoadFile(filename.string());
        if (rootNode.IsNull() || !rootNode["resources"])
        {
            return;
        }

        const auto& resourceNode = rootNode["resources"];
        if (resourceNode.IsNull())
        {
            LOG_WARN("ResourceManager - <{}> does not hold any metadata.", filename.string());
            return;
        }

        if (!resourceNode.IsSequence())
        {
            LOG_WARN("ResourceManager - 'resources' node is not a YAML Sequence. <{}>", filename.string());
            return;
        }

        for (const auto& metadataNode : resourceNode)
        {
            if (!metadataNode.IsMap())
            {
                LOG_WARN("ResourceManager - Metadata node is not a YAML Map. <{}>", filename.string());
                continue;
            }

            if (!metadataNode["id"])
            {
                LOG_WARN("ResourceManager - Metadata does not have an Id. <{}>", filename.string());
                continue;
            }
            const u64 id = metadataNode["id"].as<u64>();

            auto flagsNode = metadataNode["flags"];
            auto flags = flagsNode.as<i32>();
            UNUSED(flags);

            auto& metadata = m_metadataMap[id];
            metadata.id = id;
            metadata.binaryFile = (filename.parent_path() / metadataNode["data_bank"].as<std::string>()).string();
            metadata.binaryOffset = metadataNode["data_offset"].as<u64>();
            metadata.binarySize = metadataNode["data_size"].as<u64>();
            metadata.flags = ResourceFlags(metadataNode["flags"].as<ResourceFlags::MaskType>());

            LOG_DEBUG("ResourceManager - Metadata - Id = {}, BinaryFile = <{}>, BinaryOffset = {}, BinarySize = {}, Flags = {}",
                      metadata.id,
                      metadata.binaryFile,
                      metadata.binaryOffset,
                      metadata.binarySize,
                      static_cast<ResourceFlags::MaskType>(metadata.flags));
        }
    }

    void ResourceManager::load_resource(ResourceId id)
    {
        ASSERT(id);
        ASSERT(m_metadataMap.find(id) != m_metadataMap.end());

        if (m_pendingResources.contains(id))
        {
            return;
        }

        m_pendingResources.emplace(id);
        m_resourceLoadQueue.push(id);
        LOG_DEBUG("ResourceManager - Resource <{}> marked for loading.", id);
    }

    void ResourceManager::force_load_resource(ResourceId id)
    {
        ASSERT(id);
        ASSERT(m_metadataMap.contains(id));

        LOG_DEBUG("ResourceManager - Force loading <{}>.", id);

        auto& metadata = get_metadata(id);
        auto* factory = m_resourceFactories[metadata.typeId].get();
        auto resource = factory->load(metadata);
        if (resource == nullptr)
        {
            metadata.isLoaded = false;
            LOG_ERROR("ResourceManager - Failed to loaded resource <id = {}>!", id);
            return;
        }

        auto* cache = m_resourceCaches[metadata.typeId].get();
        cache->add(id, std::move(resource));

        metadata.isLoaded = true;
        m_pendingResources.erase(id);
    }

}
