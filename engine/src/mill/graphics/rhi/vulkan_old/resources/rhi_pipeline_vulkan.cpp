#include "rhi_pipeline_vulkan.hpp"

#include "mill/core/base.hpp"
#include "../includes_vulkan.hpp"
#include "../rhi_context_vulkan.hpp"

#include <unordered_map>

namespace mill::rhi
{
#if 0
	struct PipelineStateVulkan
    {
        hasht hash{};
        vk::Pipeline pipeline{};
    };
#endif  // 0

    std::unordered_map<hasht, vk::Pipeline> g_PipelineCache{};

    void set_pipeline_vertex_input_state(u64 context, const PipelineVertexInputState& state)
    {
        // #TODO: Get context state
        const auto* context_instance = get_context_instance(context);

        const hasht current_state_hash = context_instance->pipelineState.vertexInputStateHash;
        const auto new_state_hash = state.get_hash();

        if (new_state_hash == current_state_hash)
            return;  // No change

        // #TODO: If different then compile the vertex input interface, map it and then assign it to the current pipeline state
        // #TODO: Mark the contexts current pipeline state as dirty
    }
}
