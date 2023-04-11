#include "rhi_core_vulkan.hpp"

#include "mill/graphics/rhi.hpp"
#include "rhi_resources_vulkan.hpp"
#include "screen_vulkan.hpp"
#include "contexts/context_vulkan.hpp"
#include "resources/view_vulkan.hpp"
#include "image_vulkan.hpp"
#include "helpers_vulkan.hpp"

#include <xutility>

namespace mill::rhi
{
    Owned<InstanceVulkan> g_device{ nullptr };

    auto get_device() -> class InstanceVulkan&
    {
        return *g_device;
    }

    bool initialise()
    {
        ASSERT(g_device == nullptr);

        g_device = CreateOwned<InstanceVulkan>();
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