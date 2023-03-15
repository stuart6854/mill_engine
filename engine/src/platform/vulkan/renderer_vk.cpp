#include "renderer_vk.hpp"

#include "device_vk.hpp"
#include "context_vk.hpp"
#include "resources_vk.hpp"
#include "helpers_vk.hpp"
#include "builtin_spirv_shaders.hpp"

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

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

        {
            // Global set
            DescriptorSetLayout global_layout(m_device->get_device());
            // Buffer for global data eg. time
            global_layout.add_binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
            // Array of images for bindless image rendering
            global_layout.add_binding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 256);
            LOG_DEBUG("Global Layout hash = {}", global_layout.get_hash());
            global_layout.build();

            // Scene Set
            DescriptorSetLayout scene_layout(m_device->get_device());
            // Buffer for scene data eg. lighting
            scene_layout.add_binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
            LOG_DEBUG("Scene Layout hash = {}", scene_layout.get_hash());
            scene_layout.build();

            // Material Set
            DescriptorSetLayout material_layout(m_device->get_device());
            // Buffer for material data eg. default material data + material instance data
            material_layout.add_binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
            LOG_DEBUG("Material Layout hash = {}", material_layout.get_hash());
            material_layout.build();

            m_pipelineLayout = CreateOwned<PipelineLayout>(m_device->get_device());
            m_pipelineLayout->add_push_constant_range(vk::ShaderStageFlagBits::eVertex, 0, sizeof(CameraData));
            m_pipelineLayout->add_descriptor_set_layout(0, global_layout);
            m_pipelineLayout->add_descriptor_set_layout(1, scene_layout);
            m_pipelineLayout->add_descriptor_set_layout(2, material_layout);
            LOG_DEBUG("Pipeline hash = {}", m_pipelineLayout->get_hash());
            m_pipelineLayout->build();

            PipelineInit pipeline_info{};
            pipeline_info.layout = m_pipelineLayout.get();
            pipeline_info.vertexSrc = g_ShaderDefaultSrc_Vertex;
            pipeline_info.fragmentSrc = g_ShaderDefaultSrc_Fragment;
            pipeline_info.state.cullMode = vk::CullModeFlagBits::eNone;
            m_pipeline = m_device->create_pipeline(pipeline_info);
        }

        const f32 aspect_ratio = static_cast<f32>(m_displaySize.x) / static_cast<f32>(m_displaySize.y);
        m_cameraData.projection = glm::perspective(glm::radians(60.0f), aspect_ratio, 0.1f, 100.0f);
        m_cameraData.view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 5.0f));
        m_cameraData.view = glm::inverse(m_cameraData.view);
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
        m_graphicsContext->set_constants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(CameraData), &m_cameraData);
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