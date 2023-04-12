#pragma once

#include "mill/core/base.hpp"
#include "resources/rhi_pipeline_vulkan.hpp"
#include "vulkan_includes.hpp"

namespace mill::rhi
{
    struct PipelineState
    {
        hasht vertexInputStateHash{};
        hasht preRasterisationStateHash{};
        hasht fragmentStageStateHash{};
        hasht fragmentOutputStateHash{};

        auto get_hash() const -> hasht;
    };

    auto to_vulkan(Format format) -> vk::Format;

}
