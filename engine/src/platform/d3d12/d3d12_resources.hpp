#pragma once

#include "mill/core/base.hpp"
#include "d3d12_common_headers.hpp"

namespace mill::platform
{
    struct DescriptorD3D12
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};
        u32 heapIndex{};
    };

    enum class ResourceTypeD3D12
    {
        eNone,
        eBuffer,
        eTexture,
    };

    struct ResourceD3D12
    {
        ResourceTypeD3D12 type{};
        ID3D12Resource* resource{ nullptr };
        D3D12_RESOURCE_STATES state{};
        u32 descriptorHeapIndex{ 0xffffffff };
    };

    struct TextureResourceD3D12 : public ResourceD3D12
    {
        TextureResourceD3D12() : ResourceD3D12()
        {
            type = ResourceTypeD3D12::eTexture;
        }

        DescriptorD3D12 rtvDescriptor{};
    };
}