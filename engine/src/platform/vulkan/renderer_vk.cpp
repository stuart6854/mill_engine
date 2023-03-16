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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

        // Depth Image
        {
            ImageInit image_init{};
            image_init.width = 1600;
            image_init.height = 900;
            image_init.format = vk::Format::eD32Sfloat;
            image_init.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
            m_depthImage = m_device->create_image(image_init);
        }

        {
            struct Vertex
            {
                glm::vec3 position{};
                glm::vec4 color{};
            };
            std::vector<Vertex> vertices{
                { { 0.0f, 0.0f, -0.5f }, { 1.0f, 0.0f, .0f, 1.0f } },
                { { -0.5f, 0.0f, 0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
                { { 0.5f, 0.0f, 0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
                { { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
            };

            BufferInit vertex_buffer_init{};
            vertex_buffer_init.size = vec_data_size(vertices);
            vertex_buffer_init.usage = vk::BufferUsageFlagBits::eVertexBuffer;
            vertex_buffer_init.initial_data = vertices.data();
            m_vertexBuffer = m_device->create_buffer(vertex_buffer_init);

            std::vector<u16> indices{ 0, 1, 2, 0, 1, 3, 1, 2, 3, 2, 0, 3 };

            BufferInit index_buffer_init{};
            index_buffer_init.size = vec_data_size(indices);
            index_buffer_init.usage = vk::BufferUsageFlagBits::eIndexBuffer;
            index_buffer_init.initial_data = indices.data();
            m_indexBuffer = m_device->create_buffer(index_buffer_init);
        }

        {
            // Global set
            m_globalSetLayout = CreateOwned<vulkan::DescriptorSetLayout>(m_device->get_device());
            // Buffer for global data eg. time
            m_globalSetLayout->add_binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
            // Array of images for bindless image rendering
            m_globalSetLayout->add_binding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 256);
            LOG_DEBUG("Global Set Layout hash = {}", m_globalSetLayout->get_hash());
            m_globalSetLayout->build();

            // Scene Set
            m_sceneSetLayout = CreateOwned<DescriptorSetLayout>(m_device->get_device());
            // Buffer for scene vertex data eg. camera matrices
            m_sceneSetLayout->add_binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
            // Buffer for scene fragment data eg. lighting
            m_sceneSetLayout->add_binding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
            LOG_DEBUG("Scene Set Layout hash = {}", m_sceneSetLayout->get_hash());
            m_sceneSetLayout->build();
        }

        {
            // Material Set
            DescriptorSetLayout material_layout(m_device->get_device());
            // Buffer for material data eg. default material data + material instance data
            material_layout.add_binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
            LOG_DEBUG("Material Layout hash = {}", material_layout.get_hash());
            material_layout.build();

            m_pipelineLayout = CreateOwned<PipelineLayout>(m_device->get_device());
            m_pipelineLayout->add_push_constant_range(vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants));
            m_pipelineLayout->add_descriptor_set_layout(0, *m_globalSetLayout);
            m_pipelineLayout->add_descriptor_set_layout(1, *m_sceneSetLayout);
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
        m_sceneData.camera_proj = glm::perspective(glm::radians(60.0f), aspect_ratio, 0.1f, 100.0f);
        m_sceneData.camera_view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
        m_sceneData.camera_view = glm::inverse(m_sceneData.camera_view);

        // Setup frames
        for (auto& frame : m_frames)
        {
            BufferInit global_ubo_info{};
            global_ubo_info.size = sizeof(GlobalData);
            global_ubo_info.usage = vk::BufferUsageFlagBits::eUniformBuffer;
            global_ubo_info.isCPUVisible = true;
            global_ubo_info.initial_data = &m_globalData;
            frame.globalUBO = m_device->create_buffer(global_ubo_info);

            frame.globalSet = m_device->create_descriptor_set(*m_globalSetLayout);
            frame.globalSet->bind_buffer(0, frame.globalUBO->buffer, sizeof(GlobalData));
            frame.globalSet->flush_writes();

            BufferInit scene_ubo_info{};
            scene_ubo_info.size = sizeof(SceneData);
            scene_ubo_info.usage = vk::BufferUsageFlagBits::eUniformBuffer;
            scene_ubo_info.isCPUVisible = true;
            scene_ubo_info.initial_data = &m_sceneData;
            frame.sceneUBO = m_device->create_buffer(scene_ubo_info);

            frame.sceneSet = m_device->create_descriptor_set(*m_sceneSetLayout);
            frame.sceneSet->bind_buffer(0, frame.sceneUBO->buffer, sizeof(SceneData));
            frame.sceneSet->flush_writes();
        }

        // Texture
        {
            const auto* texture_filename = "../../assets/textures/uv_map.png";
            i32 w, h, c;
            u8* data = stbi_load(texture_filename, &w, &h, &c, 4);
            u64 data_size = w * h * (sizeof(u8) * 4);

            ImageInit image_init{};
            image_init.width = w;
            image_init.height = h;
            image_init.format = vk::Format::eR8G8B8A8Srgb;
            image_init.usage = vk::ImageUsageFlagBits::eSampled;
            m_texture = m_device->create_image(image_init, data_size, data);

            STBI_FREE(data);
        }
    }

    void RendererVulkan::shutdown()
    {
        m_device->wait_idle();

        m_device->destroy_image(m_texture);

        for (auto& frame : m_frames)
        {
            m_device->destroy_buffer(frame.globalUBO);
            frame.globalSet = nullptr;
            m_device->destroy_buffer(frame.sceneUBO);
            frame.sceneSet = nullptr;
        }

        m_globalSetLayout = nullptr;
        m_sceneSetLayout = nullptr;

        m_device->destroy_buffer(m_indexBuffer);
        m_device->destroy_buffer(m_vertexBuffer);
        m_device->destroy_pipeline(std::move(m_pipeline));

        m_device->destroy_image(m_depthImage);

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

        m_frameIndex = (m_frameIndex + 1) % g_FrameBufferCount;
        const auto& frame = m_frames[m_frameIndex];

        m_device->begin_frame();

        m_globalData.time += 1.0f / 240.0f;
        frame.globalUBO->write(0, sizeof(GlobalData), &m_globalData);
        frame.sceneUBO->write(0, sizeof(SceneData), &m_sceneData);

        m_graphicsContext->begin_frame();

        m_graphicsContext->set_default_viewport_and_scissor({ 1600, 900 });

        auto& back_buffer = *m_device->get_current_back_buffer(m_surfaceHandle);
        m_graphicsContext->add_barrier(back_buffer, vk::ImageLayout::eAttachmentOptimal);
        m_graphicsContext->add_barrier(*m_depthImage, vk::ImageLayout::eAttachmentOptimal);

        glm::vec4 clear_color = { 0.36f, 0.54f, 0.86f, 1.0f };
        m_graphicsContext->begin_render_pass(back_buffer, &clear_color, m_depthImage);

        m_graphicsContext->set_pipeline(*m_pipeline);
        m_graphicsContext->set_descriptor_set(0, *frame.globalSet);
        m_graphicsContext->set_descriptor_set(1, *frame.sceneSet);

        m_graphicsContext->set_index_buffer(*m_indexBuffer, vk::IndexType::eUint16);
        m_graphicsContext->set_vertex_buffer(*m_vertexBuffer);

        m_pushConstants.transform = glm::rotate(m_pushConstants.transform, (1.0f / 240.0f) * glm ::radians(5.0f), glm::vec3(0, 1, 0));
        m_graphicsContext->set_constants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), &m_pushConstants);
        m_graphicsContext->draw_indexed(3 * 4, 0, 0);

        m_graphicsContext->end_render_pass();

        m_graphicsContext->add_barrier(back_buffer, vk::ImageLayout::ePresentSrcKHR);

        m_graphicsContext->end_frame();
        auto receipt = m_device->submit_context(*m_graphicsContext, nullptr);
        m_device->end_frame();
        m_device->present(&receipt);
    }

    auto RendererVulkan::get_frame() -> Frame&
    {
        return m_frames[m_frameIndex];
    }

}

namespace mill::platform
{
    auto create_renderer() -> Owned<RendererInterface>
    {
        return CreateOwned<vulkan::RendererVulkan>();
    }
}