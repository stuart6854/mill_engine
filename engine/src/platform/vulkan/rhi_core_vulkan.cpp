#include "device_vulkan.hpp"

#include "mill/platform/rhi.hpp"
#include "rhi_resources_vulkan.hpp"
#include "screen_vulkan.hpp"
#include "context_vulkan.hpp"
#include "view_vulkan.hpp"
#include "image_vulkan.hpp"
#include "helpers_vulkan.hpp"

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
                auto& semaphore = screen->get_acquire_semaphore();
                wait_semaphores.push_back(semaphore);

                present_wait_semaphore_map[screen_id] = context->get_completed_semaphore();
            }

            vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            vk::SubmitInfo submit_info{};
            submit_info.setCommandBuffers(context->get_cmd());
            submit_info.setWaitDstStageMask(wait_stage);
            submit_info.setWaitSemaphores(wait_semaphores);
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
            image_indices.push_back(screen->get_image_index());
            swap_chains.push_back(screen->get_swap_chain());
            if (present_wait_semaphore_map.contains(id))
            {
                wait_semaphores.push_back(present_wait_semaphore_map[id]);
            }
        }

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

    void blit_to_screen(u64 context, u64 screen, u64 view)
    {
        ASSERT(g_device);
        ASSERT(g_contexts);
        ASSERT(g_contexts->contextMap.contains(context));
        ASSERT(get_resources().screenMap.contains(screen));
        ASSERT(g_contexts->viewMap.contains(view));

        auto& context_instance = g_contexts->contextMap[context];
        context_instance->associate_screen(screen);

        auto& screen_inst = get_resources().screenMap.at(screen);
        auto& view_instance = g_contexts->viewMap[view];

        screen_inst->get_backbuffer().transition_to_transfer_dst(*context_instance);
        view_instance->get_color_attachment()->transition_to_transfer_src(*context_instance);
        context_instance->blit(*view_instance->get_color_attachment(), screen_inst->get_backbuffer());
        screen_inst->get_backbuffer().transition_to_present(*context_instance);
    }

    void reset_view(u64 view, u32 width, u32 height)
    {
        ASSERT(g_contexts);

        if (!g_contexts->viewMap.contains(view))
        {
            g_contexts->viewMap[view] = CreateOwned<ViewVulkan>(*g_device);
        }

        auto& view_inst = g_contexts->viewMap.at(view);
        view_inst->reset(width, height);
    }
}