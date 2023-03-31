#include "device_vulkan.hpp"

#include "mill/platform/rhi.hpp"
#include "rhi_resources_vulkan.hpp"
#include "screen_vulkan.hpp"
#include "context_vulkan.hpp"
#include "view_vulkan.hpp"
#include "image_vulkan.hpp"
#include "helpers_vulkan.hpp"

#include <xutility>

namespace mill::rhi
{
    Owned<DeviceVulkan> g_device{ nullptr };
    struct ContextsVulkan
    {
        std::unordered_map<u64, Owned<ContextVulkan>> contextMap{};
        std::unordered_map<u64, Owned<ViewVulkan>> viewMap{};
    };
    Owned<ContextsVulkan> g_contexts{ nullptr };

    auto get_device() -> class DeviceVulkan&
    {
        return *g_device;
    }

    bool initialise()
    {
        ASSERT(g_device == nullptr);

        g_device = CreateOwned<DeviceVulkan>();
        if (!g_device->initialise())
        {
            shutdown();
            return false;
        }

        g_contexts = CreateOwned<ContextsVulkan>();

        initialise_resources();

        return true;
    }

    void shutdown()
    {
        ASSERT(g_device);

        g_device->get_device().waitIdle();

        shutdown_resources();

        g_contexts.reset();

        g_device->shutdown();
        g_device.reset();
    }

    void begin_frame()
    {
        ASSERT(g_device);

        auto& screens = get_resources().screenMap;
        for (auto& [id, screen] : screens)
        {
            screen->acquire_image();
        }
    }

    void end_frame()
    {
        ASSERT(g_device);

        // Can any swap chains be presented
        bool can_present = false;
        for (auto& [id, screen] : get_resources().screenMap)
        {
            if (!screen->is_minimized())
            {
                can_present = true;
                break;
            }
        }

        std::unordered_map<u64, vk::Semaphore> present_wait_semaphore_map{};

        // Submit
        for (auto& [id, context] : g_contexts->contextMap)
        {
            if (!context->get_was_recorded())
                continue;

            std::vector<vk::Semaphore> wait_semaphores{};
            for (auto screen_id : context->get_associated_screens())
            {
                auto& screen = get_resources().screenMap.at(screen_id);
                if (screen->is_minimized())
                    continue;

                auto& semaphore = screen->get_acquire_semaphore();
                wait_semaphores.push_back(semaphore);

                present_wait_semaphore_map[screen_id] = context->get_completed_semaphore();
            }

            vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            vk::SubmitInfo submit_info{};
            submit_info.setCommandBuffers(context->get_cmd());
            submit_info.setWaitDstStageMask(wait_stage);
            submit_info.setWaitSemaphores(wait_semaphores);
            if (can_present)
                submit_info.setSignalSemaphores(context->get_completed_semaphore());
            g_device->get_graphics_queue().submit(submit_info, context->get_fence());
        }

        // Present
        std::vector<u32> image_indices{};
        std::vector<vk::SwapchainKHR> swap_chains{};
        std::vector<vk::Semaphore> wait_semaphores{};

        auto& screens = get_resources().screenMap;
        for (auto& [id, screen] : screens)
        {
            // Handle minimized screens
            if (screen->is_minimized())
                continue;

            image_indices.push_back(screen->get_image_index());
            swap_chains.push_back(screen->get_swap_chain());
            if (present_wait_semaphore_map.contains(id))
            {
                wait_semaphores.push_back(present_wait_semaphore_map[id]);
            }
        }

        // Do not Present if not any swap chains
        if (swap_chains.empty())
            return;

        vk::PresentInfoKHR present_info{};
        present_info.setImageIndices(image_indices);
        present_info.setSwapchains(swap_chains);
        present_info.setWaitSemaphores(wait_semaphores);
        auto result = g_device->get_graphics_queue().presentKHR(present_info);
        ASSERT(result == vk::Result::eSuccess);
    }

    void begin_contex(u64 context)
    {
        ASSERT(g_device);
        ASSERT(g_contexts);

        auto it = g_contexts->contextMap.find(context);
        if (it == g_contexts->contextMap.end())
        {
            // Create new context
            g_contexts->contextMap[context] = CreateOwned<ContextVulkan>(*g_device);
        }

        auto& context_instance = g_contexts->contextMap[context];
        context_instance->wait_and_begin();
    }

    void end_context(u64 context)
    {
        ASSERT(g_device);
        ASSERT(g_contexts);
        ASSERT(g_contexts->contextMap.contains(context));

        auto& context_instance = g_contexts->contextMap[context];
        context_instance->end();
    }

    void begin_view(u64 context, u64 view, const glm::vec4& clear_color, f32 clear_depth)
    {
        ASSERT(g_device);
        ASSERT(g_contexts);
        ASSERT(g_contexts->contextMap.contains(context));
        ASSERT(g_contexts->viewMap.contains(view));

        auto& context_instance = g_contexts->contextMap[context];
        auto& view_instance = g_contexts->viewMap[view];

        view_instance->get_color_attachment()->transition_to_attachment(*context_instance);

        view_instance->set_clear_color(vk::ClearColorValue(clear_color.x, clear_color.y, clear_color.z, clear_color.w));
        view_instance->set_clear_depth_stencil(vk::ClearDepthStencilValue(clear_depth, 0));

        view_instance->begin(context_instance->get_cmd());
    }

    void end_view(u64 context, u64 view)
    {
        ASSERT(g_device);
        ASSERT(g_contexts);
        ASSERT(g_contexts->contextMap.contains(context));
        ASSERT(g_contexts->viewMap.contains(view));

        auto& context_instance = g_contexts->contextMap[context];
        auto& view_instance = g_contexts->viewMap[view];
        view_instance->end(context_instance->get_cmd());
    }

    void set_viewport(u64 context, f32 x, f32 y, f32 w, f32 h, f32 min_depth, f32 max_depth)
    {
        ASSERT(g_device);
        ASSERT(g_contexts);
        ASSERT(g_contexts->contextMap.contains(context));

        auto& context_instance = g_contexts->contextMap[context];
        context_instance->set_viewport(x, y, w, h, min_depth, max_depth);
    }

    void set_scissor(u64 context, i32 x, i32 y, u32 w, u32 h)
    {
        ASSERT(g_device);
        ASSERT(g_contexts);
        ASSERT(g_contexts->contextMap.contains(context));

        auto& context_instance = g_contexts->contextMap[context];
        context_instance->set_scissor(x, y, w, h);
    }

    void set_pipeline(u64 context, HandlePipeline pipeline)
    {
        ASSERT(g_device);
        ASSERT(g_contexts);
        ASSERT(g_contexts->contextMap.contains(context));

        auto& context_instance = g_contexts->contextMap[context];
        context_instance->set_pipeline(pipeline);
    }

    void set_vertex_buffer(u64 context, HandleBuffer buffer)
    {
        ASSERT(g_device);
        ASSERT(g_contexts);
        ASSERT(g_contexts->contextMap.contains(context));

        auto& context_instance = g_contexts->contextMap[context];
        context_instance->set_vertex_buffer(buffer);
    }

    void draw(u64 context, u32 vertex_count)
    {
        ASSERT(g_device);
        ASSERT(g_contexts);
        ASSERT(g_contexts->contextMap.contains(context));

        auto& context_instance = g_contexts->contextMap[context];
        context_instance->draw(vertex_count);
    }

    void blit_to_screen(u64 context, u64 screen, u64 view)
    {
        ASSERT(g_device);
        ASSERT(g_contexts);
        ASSERT(g_contexts->contextMap.contains(context));
        ASSERT(get_resources().screenMap.contains(screen));
        ASSERT(g_contexts->viewMap.contains(view));

        auto& context_instance = g_contexts->contextMap[context];

        auto& screen_inst = get_resources().screenMap.at(screen);
        // Handle minimized screens
        if (screen_inst->is_minimized())
            return;

        auto& view_instance = g_contexts->viewMap[view];

        screen_inst->get_backbuffer().transition_to_transfer_dst(*context_instance);
        view_instance->get_color_attachment()->transition_to_transfer_src(*context_instance);
        context_instance->blit(*view_instance->get_color_attachment(), screen_inst->get_backbuffer());
        screen_inst->get_backbuffer().transition_to_present(*context_instance);

        context_instance->associate_screen(screen);
    }

    void reset_view(u64 view, u32 width, u32 height)
    {
        ASSERT(g_contexts);

        if (!g_contexts->viewMap.contains(view))
        {
            g_contexts->viewMap[view] = CreateOwned<ViewVulkan>(*g_device);
        }

        // Ensure we never have a view with a dimension of (0,0), eg. Minimized Window
        width = std::max(1u, width);
        height = std::max(1u, height);

        auto& view_inst = g_contexts->viewMap.at(view);
        view_inst->reset(width, height);
    }
}