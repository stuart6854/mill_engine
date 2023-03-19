#pragma once

#include "mill/core/base.hpp"
#include "mill/utility/flags.hpp"
#include "mill/utility/ref_count.hpp"

#include <string>

namespace mill
{
    enum class ResourceType : u8
    {
        eNone,
        eStaticMesh,
        eSkeletalMesh,
        eAudio,
    };

    enum class ResourceFlagBits : u8
    {
        eKeepLoaded = 1u << 0,                    // Keep resource loaded into memory even when nothing references it
        eEngineResource = 1u << 1 | eKeepLoaded,  // Resource is used by the engine, implies 'eKeepLoaded'
    };
    using ResourceFlags = Flags<ResourceFlagBits>;

    using ResourceId = u64;

    struct ResourceMetadata
    {
        ResourceId id{};
        std::string binaryFile{};  // The binary file the resource is loaded from
        u64 binaryOffset{};
        u64 binarySize{};
        ResourceType type{};
        ResourceFlags flags{};

        bool isLoaded{ false };
        RefCount refCount{};
    };

    class Resource
    {
    public:
        virtual ~Resource() = default;
    };

}