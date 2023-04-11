#include "mill/graphics/rhi/rhi_context.hpp"

#include "mill/core/base.hpp"
#include "mill/core/debug.hpp"
#include "rhi_core_vulkan.hpp"
#include "vulkan_device.hpp"
#include "vulkan_screen.hpp"
#include "vulkan_context.hpp"
#include "vulkan_view.hpp"

namespace mill::rhi
{
    void begin_context(u64 context_id)
    {
        auto& device = get_device();
        auto* context = device.get_context(context_id);
        if (context == nullptr)
        {
            device.create_context(context_id);
            context = device.get_context(context_id);
        }
        ASSERT(context != nullptr);
        context->wait_and_begin();
    }

    void end_context(u64 context_id)
    {
        auto& device = get_device();
        auto* context = device.get_context(context_id);
        ASSERT(context != nullptr);
        context->end();
    }

    void begin_view(u64 context_id, u64 view_id, const glm::vec4& clear_color, f32 clear_depth)
    {
        auto& device = get_device();

        auto* context = device.get_context(context_id);
        ASSERT(context != nullptr);

        auto* view = device.get_view(view_id);
        ASSERT(view != nullptr);

        // #TODO: Transition view to Attachment

        auto vk_clear_color = vk::ClearColorValue(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        view->set_clear_color(vk_clear_color);

        auto vk_clear_depth_stencil = vk::ClearDepthStencilValue(clear_depth, 0);
        view->set_clear_depth_stencil(vk_clear_depth_stencil);

        auto& cmd = context->get_cmd();
        view->begin(cmd);
    }

    void end_view(u64 context_id, u64 view_id)
    {
        auto& device = get_device();

        auto* context = device.get_context(context_id);
        ASSERT(context != nullptr);

        auto* view = device.get_view(view_id);
        ASSERT(view != nullptr);

        auto& cmd = context->get_cmd();
        view->end(cmd);
    }

    void set_viewport(u64 /*context*/, f32 /*x*/, f32 /*y*/, f32 /*w*/, f32 /*h*/, f32 /*min_depth*/, f32 /*max_depth*/) {}

    void set_scissor(u64 /*context*/, i32 /*x*/, i32 /*y*/, u32 /*w*/, u32 /*h*/) {}

    void set_index_buffer(u64 /*context*/, HandleBuffer /*buffer*/, IndexType /*index_type*/) {}

    void set_vertex_buffer(u64 /*context*/, HandleBuffer /*buffer*/) {}

    void draw(u64 /*context*/, u32 /*vertex_count*/) {}

    void draw_indexed(u64 /*context*/, u32 /*index_count*/) {}

    void blit_to_screen(u64 context_id, u64 screen_id, u64 view_id)
    {
        auto& device = get_device();

        auto* context = device.get_context(context_id);
        ASSERT(context != nullptr);

        auto* screen = device.get_screen(screen_id);
        ASSERT(screen != nullptr);

        auto* view = device.get_view(view_id);
        ASSERT(view != nullptr);

        if (screen->is_minimized())
            return;

        context->transition_image(screen->get_backbuffer(), vk::ImageLayout::eTransferDstOptimal);
        context->transition_image(*view->get_color_attachment(), vk::ImageLayout::eTransferSrcOptimal);

        context->blit(*view->get_color_attachment(), screen->get_backbuffer());

        context->transition_image(screen->get_backbuffer(), vk::ImageLayout::ePresentSrcKHR);

        context->associate_screen(screen_id);
    }
}