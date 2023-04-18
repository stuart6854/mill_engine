#pragma once

#include "mill/core/base.hpp"
#include "rhi_resource_vulkan.hpp"
#include "vulkan_includes.hpp"

#include <vector>
#include <unordered_map>

namespace mill::rhi
{
    class ScreenVulkan;
    class ContextVulkan;
    class ViewVulkan;

    class DeviceVulkan
    {
    public:
        DeviceVulkan();
        ~DeviceVulkan();

        bool initialise();
        void shutdown();
        void wait_idle() const;

#pragma region Resources

        void create_screen(u64 screen_id, void* window_handle);
        void destroy_screen(u64 screen_id);
        auto get_screen(u64 screen_id) const -> ScreenVulkan*;
        auto get_all_screens() const -> const std::vector<ScreenVulkan*>&;

        void create_context(u64 context_id);
        void destroy_context(u64 context_id);
        auto get_context(u64 context_id) const -> ContextVulkan*;
        auto get_all_contexts() const -> const std::vector<ContextVulkan*>&;

        void create_view(u64 view_id);
        void destroy_view(u64 view_id);
        auto get_view(u64 view_id) const -> ViewVulkan*;





        /* Pipelines */

        auto get_or_create_pipeline_vertex_input_module(const PipelineVertexInputStateVulkan& state) -> Shared<PipelineModuleVertexInput>;
        auto get_or_create_pipeline_pre_rasterisation_module(const PipelinePreRasterisationStateVulkan& state)
            -> Shared<PipelineModulePreRasterisation>;
        auto get_or_create_pipeline_fragment_stage_module(const PipelineFragmentStageStateVulkan& state)
            -> Shared<PipelineModuleFragmentStage>;
        auto get_or_create_pipeline_fragment_output_module(const PipelineFragmentOutputStateVulkan& state)
            -> Shared<PipelineModuleFragmentOutput>;

        auto get_or_create_pipeline(const std::vector<Shared<PipelineModule>>& modules) -> Shared<Pipeline>;
        auto get_pipeline(hasht pipeline_hash) -> Shared<Pipeline>;

#pragma endregion

        /* Getters */

        auto get_instance() -> vk::Instance&;
        auto get_physical_device() -> vk::PhysicalDevice&;
        auto get_device() -> vk::Device&;

        auto get_graphics_queue_family() -> i32;
        auto get_compute_queue_family() -> i32;
        auto get_transfer_queue_family() -> i32;

        auto get_graphics_queue() -> vk::Queue&;
        auto get_compute_queue() -> vk::Queue&;
        auto get_transfer_queue() -> vk::Queue&;

        auto get_allocator() -> vma::Allocator&;
        auto get_descriptor_pool() -> vk::DescriptorPool&;

    private:
        bool init_instance();
        bool init_physical_device();
        bool init_device();

        auto reflect_shader_stage_pipeline_layout(const std::vector<u32>& spirv, vk::ShaderStageFlagBits shader_stage)
            -> Shared<PipelineLayout>;

    private:
        vk::DynamicLoader m_loader{};
        vk::UniqueInstance m_instance;
        vk::UniqueDebugUtilsMessengerEXT m_debugMessenger{};
        vk::PhysicalDevice m_physicalDevice;
        vk::UniqueDevice m_device{};

        i32 m_graphicsQueueFamily{};
        vk::Queue m_graphicsQueue{};
        i32 m_computeQueueFamily{};
        vk::Queue m_computeQueue{};
        i32 m_transferQueueFamily{};
        vk::Queue m_transferQueue{};

        vma::UniqueAllocator m_allocator{};
        vk::UniqueDescriptorPool m_descriptorPool{};

        std::unordered_map<u64, Owned<ScreenVulkan>> m_screens{};
        std::vector<ScreenVulkan*> m_allScreens{};

        std::unordered_map<u64, Owned<ContextVulkan>> m_contexts{};
        std::vector<ContextVulkan*> m_allContexts{};

        std::unordered_map<u64, Owned<ViewVulkan>> m_views{};

        std::unordered_map<hasht, Shared<PipelineLayout>> m_pipelineLayouts{};

        std::unordered_map<hasht, Shared<PipelineModuleVertexInput>> m_vertexInputPipelineModules{};
        std::unordered_map<hasht, Shared<PipelineModulePreRasterisation>> m_preRasterisationPipelineModules{};
        std::unordered_map<hasht, Shared<PipelineModuleFragmentStage>> m_fragmentStagePipelineModules{};
        std::unordered_map<hasht, Shared<PipelineModuleFragmentOutput>> m_fragmentOutputPipelineModules{};
        std::unordered_map<hasht, Shared<Pipeline>> m_compiledPipelines{};
    };
}