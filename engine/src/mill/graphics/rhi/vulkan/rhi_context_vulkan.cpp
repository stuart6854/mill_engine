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

    void set_viewport(u64 context_id, f32 x, f32 y, f32 w, f32 h, f32 min_depth, f32 max_depth)
    {
        auto& device = get_device();

        auto* context = device.get_context(context_id);
        ASSERT(context != nullptr);

        context->set_viewport(x, y, w, h, min_depth, max_depth);
    }

    void set_scissor(u64 context_id, i32 x, i32 y, u32 w, u32 h)
    {
        auto& device = get_device();

        auto* context = device.get_context(context_id);
        ASSERT(context != nullptr);

        context->set_scissor(x, y, w, h);
    }

    void set_pipeline(u64 context_id, u64 pipeline_id)
    {
        auto& device = get_device();

        auto* context = device.get_context(context_id);
        ASSERT(context != nullptr);

        context->set_pipeline(pipeline_id);
    }

    void set_index_buffer(u64 context_id, HandleBuffer buffer_id, IndexType index_type)
    {
        auto& device = get_device();

        auto* context = device.get_context(context_id);
        ASSERT(context != nullptr);

        auto vk_index_type = index_type == IndexType::eU16 ? vk::IndexType::eUint16 : vk::IndexType::eUint32;
        context->set_index_buffer(buffer_id, vk_index_type);
    }

    void set_vertex_buffer(u64 context_id, HandleBuffer buffer_id)
    {
        auto& device = get_device();

        auto* context = device.get_context(context_id);
        ASSERT(context != nullptr);

        context->set_vertex_buffer(buffer_id);
    }

    void set_resource_sets(u64 context_id, const std::vector<u64>& resource_set_ids)
    {
        auto& device = get_device();

        auto* context = device.get_context(context_id);
        ASSERT(context != nullptr);

        context->set_resource_sets(resource_set_ids);
    }

    void set_push_constants(u64 context_id, u32 offset, u32 size, const void* data)
    {
        auto& device = get_device();

        auto* context = device.get_context(context_id);
        ASSERT(context != nullptr);

        context->set_push_constants(offset, size, data);
    }

    void draw(u64 context_id, u32 vertex_count)
    {
        auto& device = get_device();

        auto* context = device.get_context(context_id);
        ASSERT(context != nullptr);

        context->draw(vertex_count);
    }

    void draw_indexed(u64 context_id, u32 index_count, u32 instance_count, u32 index_offset, u32 vertex_offset)
    {
        auto& device = get_device();

        auto* context = device.get_context(context_id);
        ASSERT(context != nullptr);

        context->draw_indexed(index_count, instance_count, index_offset, vertex_offset);
    }

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