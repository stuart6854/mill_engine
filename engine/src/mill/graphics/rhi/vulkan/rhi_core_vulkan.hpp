#pragma once

namespace mill::rhi
{
    constexpr u32 g_FrameBufferCount = 2;

    auto get_device() -> class DeviceVulkan&;
}
