#pragma once

#include "mill/core/base.hpp"
#include "mill/graphics/renderer.hpp"

#include <glm/ext/vector_uint2.hpp>
#include <glm/ext/matrix_float4x4.hpp>

namespace mill::platform::vulkan
{
    class DeviceVulkan;
    class ContextVulkan;
    struct PipelineVulkan;
    struct BufferVulkan;
    class PipelineLayout;
    class DescriptorSetLayout;
    class DescriptorSet;

    class RendererVulkan : public RendererInterface
    {
    public:
        RendererVulkan() = default;
        ~RendererVulkan() = default;

        DISABLE_COPY_AND_MOVE(RendererVulkan);

        void inititialise(const RendererInit& init) override;
        void shutdown() override;

        void render(const SceneInfo& scene_info) override;

    private:
        struct Frame;
        auto get_frame() -> Frame&;

    private:
        Owned<vulkan::DeviceVulkan> m_device{ nullptr };
        void* m_surfaceHandle{ nullptr };
        glm::uvec2 m_displaySize{};

        Owned<vulkan::ContextVulkan> m_graphicsContext{ nullptr };
        vulkan::BufferVulkan* m_vertexBuffer{};
        vulkan::BufferVulkan* m_indexBuffer{};

        Owned<vulkan::PipelineLayout> m_pipelineLayout{ nullptr };
        Owned<vulkan::PipelineVulkan> m_pipeline{ nullptr };

        Owned<vulkan::DescriptorSetLayout> m_globalSetLayout{ nullptr };
        Owned<vulkan::DescriptorSetLayout> m_sceneSetLayout{ nullptr };

        struct GlobalData
        {
            f32 time{};
        };
        GlobalData m_globalData{};

        struct SceneData
        {
            glm::mat4 camera_proj{ 1.0f };
            glm::mat4 camera_view{ 1.0f };
        };
        SceneData m_sceneData{};

        struct Frame
        {
            vulkan::BufferVulkan* globalUBO{ nullptr };
            Owned<vulkan::DescriptorSet> globalSet{ nullptr };

            vulkan::BufferVulkan* sceneUBO{ nullptr };
            Owned<vulkan::DescriptorSet> sceneSet{ nullptr };
        };
        std::array<Frame, 2> m_frames{};
        u32 m_frameIndex{};

        struct PushConstants
        {
            glm::mat4 transform{ 1.0f };
        };
        PushConstants m_pushConstants{};
    };

}