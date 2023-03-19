#pragma once

#include "mill/core/base.hpp"
#include "mill/utility/flags.hpp"
#include "mill/utility/ref_count.hpp"

#include <string>

namespace mill
{
    using ResourceId = u64;

    using ResourceTypeId = u32;
    constexpr ResourceTypeId ResourceType_StaticMesh = 0;
    constexpr ResourceTypeId ResourceType_SkeletalMesh = 1;
    constexpr ResourceTypeId ResourceType_Material = 2;
    constexpr ResourceTypeId ResourceType_Audio = 3;
    constexpr ResourceTypeId ResourceType_Scene = 4;

    enum class ResourceFlagBits : u8
    {
        eKeepLoaded = 1u << 0,                    // Keep resource loaded into memory even when nothing references it
        eEngineResource = 1u << 1 | eKeepLoaded,  // Resource is used by the engine, implies 'eKeepLoaded'
    };
    using ResourceFlags = Flags<ResourceFlagBits>;

    struct ResourceMetadata
    {
        ResourceId id{};
        std::string binaryFile{};  // The binary file the resource is loaded from
        u64 binaryOffset{};
        u64 binarySize{};
        ResourceTypeId typeId{};
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