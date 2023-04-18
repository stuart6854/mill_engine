#pragma once

#include "mill/core/base.hpp"
#include "../rhi_resource.hpp"

namespace mill::rhi
{
    enum class BufferUsage
    {
        eVertexBuffer,
        eIndexBuffer,
        eUniformBuffer,
        eTransferSrc,
    };

    enum class MemoryUsage
    {
        eHost,
        eDevice,
        eDeviceHostVisble,
    };

    struct BufferDescription
    {
        u64 size{};
        BufferUsage usage{};
        MemoryUsage memoryUsage{};
    };

    using HandleBuffer = u64;
    auto create_buffer(const BufferDescription& description) -> HandleBuffer;
    void write_buffer(HandleBuffer buffer_id, u64 offset, u64 size, const void* data);
}