#include "renderer_vk.hpp"

#include "device_vk.hpp"
#include "context_vk.hpp"
#include "resources_vk.hpp"
#include "builtin_spirv_shaders.hpp"

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

namespace mill::platform::vulkan
{
    void RendererVulkan::inititialise(const RendererInit& init)
    {
        AppInfo app_info{};
        app_info.appName = "app_name";

        m_device = CreateOwned<DeviceVulkan>();
        if (!m_device->initialise(app_info))
        {
            LOG_ERROR("RendererVulkan - Failed to initialise the device!");
            m_device = nullptr;
            return;
        }

        m_surfaceHandle = init.window_handle;
        m_displaySize = { init.window_width, init.window_height };
        if (!m_device->add_surface(init.window_handle, m_displaySize.x, m_displaySize.y))
        {
            LOG_ERROR("RendererVulkan - Failed to add surface to device!");
            return;
        }

        m_graphicsContext = m_device->create_context();

        PipelineInit pipeline_info{};
        pipeline_info.vertexSrc = g_ShaderTriangleSrc_Vertex;
        pipeline_info.fragmentSrc = g_ShaderTriangleSrc_Fragment;
        pipeline_info.state.cullMode = vk::CullModeFlagBits::eNone;
        m_pipeline = m_device->create_pipeline(pipeline_info);

        {
            struct Vertex
            {
                glm::vec3 position{};
                glm::vec4 color{};
            };
            std::vector<Vertex> vertices{
                { { 0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
                { { -0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
                { { 0.0f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
            };

            BufferInit vertex_buffer_init{};
            vertex_buffer_init.size = vec_data_size(vertices);
            vertex_buffer_init.usage = vk::BufferUsageFlagBits::eVertexBuffer;
            vertex_buffer_init.initial_data = vertices.data();
            m_vertexBuffer = m_device->create_buffer(vertex_buffer_init);

            std::vector<u16> indices{ 0, 1, 2 };

            BufferInit index_buffer_init{};
            index_buffer_init.size = vec_data_size(indices);
            index_buffer_init.usage = vk::BufferUsageFlagBits::eIndexBuffer;
            index_buffer_init.initial_data = indices.data();
            m_indexBuffer = m_device->create_buffer(index_buffer_init);
        }
    }

    void RendererVulkan::shutdown()
    {
        m_device->wait_idle();

        m_device->destroy_buffer(m_indexBuffer);
        m_device->destroy_buffer(m_vertexBuffer);
        m_device->destroy_pipeline(std::move(m_pipeline));
        m_device->destroy_context(std::move(m_graphicsContext));
        m_device->shutdown();
        m_device = nullptr;
    }

    void RendererVulkan::render(const SceneInfo& /*scene_info*/)
    {
        if (m_device == nullptr)
        {
            return;
        }

        m_device->begin_frame();
        m_graphicsContext->begin_frame();

        auto& back_buffer = *m_device->get_current_back_buffer(m_surfaceHandle);
        m_graphicsContext->add_barrier(back_buffer, vk::ImageLayout::eAttachmentOptimal);

        glm::vec4 clear_color = { 0.36f, 0.54f, 0.86f, 1.0f };
        m_graphicsContext->set_default_viewport_and_scissor({ 1600, 900 });
        m_graphicsContext->begin_render_pass(back_buffer, &clear_color);
        m_graphicsContext->set_pipeline(*m_pipeline);
        m_graphicsContext->set_index_buffer(*m_indexBuffer, vk::IndexType::eUint16);
        m_graphicsContext->set_vertex_buffer(*m_vertexBuffer);
        m_graphicsContext->draw_indexed(3, 0, 0);
        m_graphicsContext->end_render_pass();

        m_graphicsContext->add_barrier(back_buffer, vk::ImageLayout::ePresentSrcKHR);

        m_graphicsContext->end_frame();
        auto receipt = m_device->submit_context(*m_graphicsContext, nullptr);
        m_device->end_frame();
        m_device->present(&receipt);
    }

}

namespace mill::platform
{
    auto create_renderer() -> Owned<RendererInterface>
    {
        return CreateOwned<vulkan::RendererVulkan>();
    }
}