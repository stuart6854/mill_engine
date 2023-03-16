#pragma once

#include "mill/core/base.hpp"
#include "common_headers_vk.hpp"
#include "resources_vk.hpp"

#include <glm/ext/vector_uint2.hpp>
#include <glm/ext/vector_float4.hpp>

namespace mill::platform::vulkan
{
    class DeviceVulkan;

    class ContextVulkan
    {
    public:
        ContextVulkan(DeviceVulkan& device);
        ~ContextVulkan();

        DISABLE_COPY(ContextVulkan);

        /* Reset context to next frame and, if required, wait on its fence. Starts command buffer recording. */
        void begin_frame();

        /* Stop recording on current frame. */
        void end_frame();

        /* Common Operations */

        void add_barrier(ImageVulkan& image_resource, vk::ImageLayout new_layout);

        /* Graphics Operations */

        void begin_render_pass(ImageVulkan& target_image, const glm::vec4* clear_color);
        void end_render_pass();

        void set_pipeline(PipelineVulkan& pipeline);
        void set_descriptor_set(u32 set, DescriptorSet& descriptor_set);
        void set_viewport(const vk::Viewport& viewport);
        void set_scissor(const vk::Rect2D& scissor);
        void set_default_viewport_and_scissor(const glm::uvec2& screen_size);
        void set_index_buffer(BufferVulkan& index_buffer, vk::IndexType index_type);
        void set_vertex_buffer(BufferVulkan& vertex_buffer);
        void set_constants(vk::ShaderStageFlags stages, u32 offset_bytes, u32 size_bytes, const void* data);
        void draw(u32 vertex_count, u32 vertex_offset);
        void draw_indexed(u32 index_count, u32 index_offset, u32 vertex_offset);

        /* Getters */

        auto get_current_cmd() const -> vk::CommandBuffer;
        auto get_current_semaphore() const -> vk::Semaphore;
        auto get_current_fence() const -> vk::Fence;

    private:
        DeviceVulkan& m_device;

        struct Frame
        {
            vk::CommandPool cmdPool{};
            vk::CommandBuffer cmdBuffer{};
            vk::Semaphore semaphore{};
            vk::Fence fence{};
        };
        std::array<Frame, g_FrameBufferCount> m_frames{};
        u32 m_frameIndex{};

        PipelineVulkan* m_boundPipeline{ nullptr };
        BufferVulkan* m_boundIndexBuffer{ nullptr };
        BufferVulkan* m_boundVertexBuffer{ nullptr };
    };
}