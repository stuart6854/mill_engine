#pragma once

#include "resources/rhi_pipeline_vulkan.hpp"

namespace mill::rhi
{
    struct ContextInstanceVulkan
    {
        Owned<ContextVulkan> context{ nullptr };
        PipelineStateVulkan pipelineState{};
    };

    auto get_context_instance(u64 context) -> const ContextInstanceVulkan*;
}
