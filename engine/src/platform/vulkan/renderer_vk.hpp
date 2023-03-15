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
        Owned<vulkan::DeviceVulkan> m_device{ nullptr };
        void* m_surfaceHandle{ nullptr };
        glm::uvec2 m_displaySize{};

        Owned<vulkan::ContextVulkan> m_graphicsContext{ nullptr };
        vulkan::BufferVulkan* m_vertexBuffer{};
        vulkan::BufferVulkan* m_indexBuffer{};

        Owned<vulkan::PipelineLayout> m_pipelineLayout{ nullptr };
        Owned<vulkan::PipelineVulkan> m_pipeline{ nullptr };

        struct CameraData
        {
            glm::mat4 projection{ 1.0f };
            glm::mat4 view{ 1.0f };
        };
        CameraData m_cameraData{};
    };

}