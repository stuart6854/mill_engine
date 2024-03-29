#pragma once

#include "mill/core/base.hpp"
#include "rhi_core_vulkan.hpp"
#include "rhi_resource_vulkan.hpp"
#include "vulkan_includes.hpp"

#include <array>
#include <queue>
#include <vector>
#include <unordered_map>

namespace mill::rhi
{
    class ScreenVulkan;
    class ContextVulkan;
    class ViewVulkan;
    class DescriptorSetLayout;
    class DescriptorSet;
    class PipelineLayout;
    class PipelineModule;
    class PipelineModuleVertexInput;
    class PipelineModulePreRasterisation;
    class PipelineModuleFragmentStage;
    class PipelineModuleFragmentOutput;
    class Pipeline;
    class Buffer;
    class Sampler;
    class ImageVulkan;

    class DeviceVulkan
    {
    public:
        DeviceVulkan();
        ~DeviceVulkan();

        bool initialise();
        void shutdown();

        void next_frame();

        void wait_idle() const;

        auto begin_transfer_cmd() -> vk::CommandBuffer;
        void end_transfer_cmd_blocking(vk::CommandBuffer cmd);

        void add_deletion_func(std::function<void()>&& func);

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

        /* Resource Set */

        auto get_or_create_resource_set_layout(const ResourceSetDescriptionVulkan& description) -> Shared<DescriptorSetLayout>;

        auto create_resource_set(const ResourceSetDescriptionVulkan& description) -> u64;
        auto get_resource_set(u64 resource_set_id) -> const Shared<DescriptorSet>&;

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

        /* Buffers */

        auto get_buffer(u64 buffer_id) -> const Buffer&;
        auto create_buffer(const BufferDescriptionVulkan& description) -> u64;
        void write_buffer(u64 buffer_id, u64 offset, u64 size, const void* data);

        void destroy_buffer(u64 buffer_id);

        /* Samplers */

        auto get_or_create_sampler(const SamplerDescriptionVulkan& description) -> Shared<Sampler>;

        /* Textures */

        auto get_texture(u64 texture_id) -> const ImageVulkan&;
        auto create_texture(const TextureDescriptionVulkan& description) -> u64;
        void write_texture(u64 texture_id, u32 mip_level, const void* data);
        void generate_mip_maps(u64 texture_id);

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

        auto merge_descriptor_set_layouts(const DescriptorSetLayout& layout_a, const DescriptorSetLayout& layout_b)
            -> Shared<DescriptorSetLayout>;
        auto merge_pipeline_layouts(const PipelineLayout& layout_a, const PipelineLayout& layout_b) -> Shared<PipelineLayout>;

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

        vk::UniqueCommandPool m_transferCmdPool{};

        vma::UniqueAllocator m_allocator{};
        vk::UniqueDescriptorPool m_descriptorPool{};

        std::array<std::queue<std::function<void()>>, g_FrameBufferCount> m_destructionQueues;
        u32 m_frameIndex{};

        std::unordered_map<u64, Owned<ScreenVulkan>> m_screens{};
        std::vector<ScreenVulkan*> m_allScreens{};

        std::unordered_map<u64, Owned<ContextVulkan>> m_contexts{};
        std::vector<ContextVulkan*> m_allContexts{};

        std::unordered_map<u64, Owned<ViewVulkan>> m_views{};

        std::unordered_map<hasht, Shared<DescriptorSetLayout>> m_descriptorSetLayouts{};
        std::unordered_map<hasht, Shared<DescriptorSet>> m_descriptorSets{};
        u64 m_nextResourceSetId{ 1 };

        std::unordered_map<hasht, Shared<PipelineLayout>> m_pipelineLayouts{};
        std::unordered_map<hasht, Shared<PipelineModuleVertexInput>> m_vertexInputPipelineModules{};
        std::unordered_map<hasht, Shared<PipelineModulePreRasterisation>> m_preRasterisationPipelineModules{};
        std::unordered_map<hasht, Shared<PipelineModuleFragmentStage>> m_fragmentStagePipelineModules{};
        std::unordered_map<hasht, Shared<PipelineModuleFragmentOutput>> m_fragmentOutputPipelineModules{};
        std::unordered_map<hasht, Shared<Pipeline>> m_compiledPipelines{};

        std::unordered_map<u64, Owned<Buffer>> m_buffers{};
        u64 m_nextBufferId{ 1 };

        std::unordered_map<hasht, Shared<Sampler>> m_samplers{};

        std::unordered_map<u64, Owned<ImageVulkan>> m_textures{};
        u64 m_nextTextureId{ 1 };
    };
}