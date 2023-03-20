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
    struct ImageVulkan;
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

        void initialise(const RendererInit& init) override;
        void shutdown() override;

        void render(const SceneInfo& scene_info) override;

        auto create_static_mesh() -> Owned<StaticMesh> override;
        void destroy_static_mesh(StaticMesh* static_mesh) override;

        void wait_idle() override;

    private:
        struct Frame;
        auto get_frame() -> Frame&;

        void update_static_mesh(StaticMesh* static_mesh);

        void bind_texture_bindless(ImageVulkan& image, u32 index);

    private:
        Owned<DeviceVulkan> m_device{ nullptr };
        void* m_surfaceHandle{ nullptr };
        glm::uvec2 m_displaySize{};

        Owned<ContextVulkan> m_graphicsContext{ nullptr };

        ImageVulkan* m_depthImage{ nullptr };

        BufferVulkan* m_vertexBuffer{};
        BufferVulkan* m_indexBuffer{};

        Owned<PipelineLayout> m_pipelineLayout{ nullptr };
        Owned<PipelineVulkan> m_pipeline{ nullptr };

        ImageVulkan* m_whiteImage{ nullptr };

        Owned<DescriptorSetLayout> m_globalSetLayout{ nullptr };
        u32 m_nextTextureBindIndex{};

        Owned<DescriptorSetLayout> m_sceneSetLayout{ nullptr };

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
            BufferVulkan* globalUBO{ nullptr };
            Owned<DescriptorSet> globalSet{ nullptr };

            BufferVulkan* sceneUBO{ nullptr };
            Owned<DescriptorSet> sceneSet{ nullptr };
        };
        std::array<Frame, 2> m_frames{};
        u32 m_frameIndex{};

        struct PushConstants
        {
            glm::mat4 transform{ 1.0f };
            u32 textureId{};
        };
        PushConstants m_pushConstants{};

        ImageVulkan* m_texture{ nullptr };

        struct StaticMeshInternal
        {
            BufferVulkan* vertexBuffer{ nullptr };
            BufferVulkan* indexBuffer{ nullptr };
            u32 indexCount{};
        };
        std::unordered_map<StaticMesh*, StaticMeshInternal> m_internalStaticMeshes{};
    };

}