#pragma once

#include "mill/platform/rhi.hpp"

namespace mill::rhi
{
    struct ResourcesVulkan
    {
        std::unordered_map<u64, Owned<class ScreenVulkan>> screenMap{};

        std::unordered_map<u64, Owned<class PipelineVulkan>> pipelineMap{};
        u64 nextPipelineId{ 1 };
    };

    void initialise_resources();
    void shutdown_resources();

    auto get_resources() -> ResourcesVulkan&;

}
