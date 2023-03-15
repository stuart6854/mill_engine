#pragma once

#include "mill/core/base.hpp"
#include "mill/graphics/renderer.hpp"

#include <glm/ext/vector_uint2.hpp>

namespace mill::platform::vulkan
{
    class DeviceVulkan;
    class ContextVulkan;
    struct PipelineVulkan;
    struct BufferVulkan;

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
        Owned<vulkan::PipelineVulkan> m_pipeline{ nullptr };
        vulkan::BufferVulkan* m_vertexBuffer{};
        vulkan::BufferVulkan* m_indexBuffer{};
    };

}