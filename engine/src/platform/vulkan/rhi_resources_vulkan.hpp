#pragma once

#include "mill/platform/rhi.hpp"
#include "includes_vulkan.hpp"

namespace mill::rhi
{
    struct ResourcesVulkan
    {
        std::unordered_map<u64, Owned<class ScreenVulkan>> screenMap{};

        std::unordered_map<u64, Owned<class PipelineVulkan>> pipelineMap{};
        u64 nextPipelineId{ 1 };

        std::unordered_map<u64, Owned<class BufferVulkan>> bufferMap{};
        u64 nextBufferId{ 1 };
    };

    void initialise_resources();
    void shutdown_resources();

    auto get_resources() -> ResourcesVulkan&;

#pragma region Utility

    auto convert_topology(PrimitiveTopology topology) -> vk::PrimitiveTopology;
    auto convert_format(Format format) -> vk::Format;
    auto convert_formats(const std::vector<Format>& formats) -> std::vector<vk::Format>;

    auto get_format_size(Format format) -> u32;

#pragma endregion

}
