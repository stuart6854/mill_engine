#pragma once

#include "mill/core/base.hpp"

namespace mill::rhi
{
    constexpr u32 g_FrameBufferCount = 2;

    auto get_device() -> class InstanceVulkan&;
}
