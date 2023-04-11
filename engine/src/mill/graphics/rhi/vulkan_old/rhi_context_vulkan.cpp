#include "rhi_context_vulkan.hpp"

#include "mill/core/base.hpp"
#include "mill/core/debug.hpp"
#include "includes_vulkan.hpp"
#include "rhi_core_vulkan.hpp"
#include "rhi_context_vulkan.hpp"
#include "contexts/context_vulkan.hpp"

namespace mill::rhi
{
    struct ContextsVulkan
    {
        std::unordered_map<u64, ContextInstanceVulkan> contextMap{};
        // std::unordered_map<u64, Owned<ViewVulkan>> viewMap{};
    };
    Owned<ContextsVulkan> g_contexts{ nullptr };

    auto get_context_instance(u64 context) -> const ContextInstanceVulkan*
    {
        auto& device = get_device();
        ASSERT(g_contexts);

        auto it = g_contexts->contextMap.find(context);
        if (it == g_contexts->contextMap.end())
        {
#if 0
			// Create new context
            auto& new_context_instance = g_contexts->contextMap[context];
            new_context_instance.context = CreateOwned<ContextVulkan>(device);
#endif  // 0
            return nullptr;
        }

        const auto* context_instance = g_contexts->contextMap[context];
        return &context_instance;
    }

    void begin_contex(u64 context)
    {
        const auto* context_instance = get_context_instance(context);
        context_instance->context->wait_and_begin();
    }

    void end_context(u64 context)
    {
        const auto* context_instance = get_context_instance(context);
        context_instance->context->end();
    }

    void begin_view(u64 context, u64 view, const glm::vec4& clear_color, f32 clear_depth)
    {
        const auto* context_instance = get_context_instance(context);
        ASSERT(g_contexts->viewMap.contains(view));

        auto& view_instance = g_contexts->viewMap[view];

        view_instance->get_color_attachment()->transition_to_attachment(*context_instance);

        view_instance->set_clear_color(vk::ClearColorValue(clear_color.x, clear_color.y, clear_color.z, clear_color.w));
        view_instance->set_clear_depth_stencil(vk::ClearDepthStencilValue(clear_depth, 0));

        view_instance->begin(context_instance->context->get_cmd());
    }

    void end_view(u64 context, u64 view)
    {
        const auto* context_instance = get_context_instance(context);
        ASSERT(g_contexts->viewMap.contains(view));

        auto& view_instance = g_contexts->viewMap[view];
        view_instance->end(context_instance->context->get_cmd());
    }

    void set_viewport(u64 context, f32 x, f32 y, f32 w, f32 h, f32 min_depth, f32 max_depth)
    {
        const auto* context_instance = get_context_instance(context);
        context_instance->context->set_viewport(x, y, w, h, min_depth, max_depth);
    }

    void set_scissor(u64 context, i32 x, i32 y, u32 w, u32 h)
    {
        const auto* context_instance = get_context_instance(context);
        context_instance->context->set_scissor(x, y, w, h);
    }

    void set_index_buffer(u64 context, HandleBuffer buffer, IndexType index_type)
    {
        const auto* context_instance = get_context_instance(context);
        context_instance->context->set_index_buffer(buffer, index_type);
    }

    void set_vertex_buffer(u64 context, HandleBuffer buffer)
    {
        const auto* context_instance = get_context_instance(context);
        context_instance->context->set_vertex_buffer(buffer);
    }

    void draw(u64 context, u32 vertex_count)
    {
        const auto* context_instance = get_context_instance(context);
        context_instance->context->draw(vertex_count);
    }

    void draw_indexed(u64 context, u32 index_count)
    {
        const auto* context_instance = get_context_instance(context);
        context_instance->context->draw_indexed(index_count);
    }

    void blit_to_screen(u64 context, u64 screen, u64 view)
    {
        const auto* context_instance = get_context_instance(context);

        ASSERT(get_resources().screenMap.contains(screen));
        ASSERT(g_contexts->viewMap.contains(view));

        auto& screen_inst = get_resources().screenMap.at(screen);
        // Handle minimized screens
        if (screen_inst->is_minimized())
            return;

        auto& view_instance = g_contexts->viewMap[view];

        screen_inst->get_backbuffer().transition_to_transfer_dst(*context_instance);
        view_instance->get_color_attachment()->transition_to_transfer_src(*context_instance);
        context_instance->context->blit(*view_instance->get_color_attachment(), screen_inst->get_backbuffer());
        screen_inst->get_backbuffer().transition_to_present(*context_instance);

        context_instance->context->associate_screen(screen);
    }

}
