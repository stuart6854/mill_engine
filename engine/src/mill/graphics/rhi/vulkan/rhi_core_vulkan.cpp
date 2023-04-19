#include "mill/graphics/rhi/rhi_core.hpp"

#include "mill/core/debug.hpp"
#include "vulkan_device.hpp"
#include "vulkan_screen.hpp"
#include "vulkan_context.hpp"
#include "vulkan_image.hpp"

#include <unordered_map>

namespace mill::rhi
{
    Owned<DeviceVulkan> g_Device{ nullptr };

    auto get_device() -> DeviceVulkan&
    {
        ASSERT(g_Device != nullptr);
        return *g_Device;
    }

    bool initialise()
    {
        ASSERT(g_Device == nullptr);

        g_Device = CreateOwned<DeviceVulkan>();
        if (!g_Device->initialise())
        {
            LOG_ERROR("RHI - Vulkan - Failed to create device!");
            return false;
        }

        ASSERT(g_Device != nullptr);
        return true;
    }

    void shutdown()
    {
        ASSERT(g_Device != nullptr);

        g_Device->shutdown();
        g_Device.reset(nullptr);

        ASSERT(g_Device == nullptr);
    }

    void begin_frame()
    {
        ASSERT(g_Device != nullptr);

        auto& screens = g_Device->get_all_screens();
        for (auto* screen : screens)
        {
            screen->acquire_image();
        }

        g_Device->next_frame();
    }

    void end_frame()
    {
        ASSERT(g_Device != nullptr);
        ASSERT(g_Device->get_graphics_queue());
        const auto& queue = g_Device->get_graphics_queue();

        // #TODO: Present screens

        // Can any swap chains be presented
        bool can_present = false;
        for (auto* screen : g_Device->get_all_screens())
        {
            if (!screen->is_minimized())
            {
                can_present = true;
                break;
            }
        }

        std::unordered_map<u64, vk::Semaphore> present_wait_semaphore_map{};

        // Submit
        const auto& contexts = g_Device->get_all_contexts();
        for (auto* context : contexts)
        {
            if (!context->get_was_recorded())
                continue;

            std::vector<vk::Semaphore> wait_semaphores{};
            for (auto& screen_id : context->get_associated_screen_ids())
            {
                auto* screen = g_Device->get_screen(screen_id);
                ASSERT(screen != nullptr);
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
            queue.submit(submit_info, context->get_fence());
        }

        // Present
        std::vector<u32> present_image_indices{};
        std::vector<vk::SwapchainKHR> present_swap_chains{};
        std::vector<vk::Semaphore> present_wait_semaphores{};

        auto& screens = g_Device->get_all_screens();
        for (auto* screen : screens)
        {
            if (screen->is_minimized())
                continue;

            present_image_indices.push_back(screen->get_image_index());
            present_swap_chains.push_back(screen->get_swap_chain());

            const auto screen_id = screen->get_id();
            ASSERT(present_wait_semaphore_map.contains(screen_id));
            present_wait_semaphores.push_back(present_wait_semaphore_map[screen_id]);
        }

        // Do not Present if not any swap chains
        if (present_swap_chains.empty())
            return;

        vk::PresentInfoKHR present_info{};
        present_info.setImageIndices(present_image_indices);
        present_info.setSwapchains(present_swap_chains);
        present_info.setWaitSemaphores(present_wait_semaphores);
        auto result = queue.presentKHR(present_info);
        ASSERT(result == vk::Result::eSuccess);
    }
}