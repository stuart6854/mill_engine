#pragma once

#include "mill/core/base.hpp"
#include "mill/graphics/rhi/resources/rhi_pipeline.hpp"
#include "../includes_vulkan.hpp"

namespace mill::rhi
{
    struct PipelineStateVulkan
    {
        PipelineVertexInputState vertexInputState{};
        u64 vertexInputStateHash{};

        u64 pipelineHash{};
    };
}