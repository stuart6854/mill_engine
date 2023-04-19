#pragma once

#include "mill/core/base.hpp"
#include "static_vertex.hpp"
#include "mill/graphics/rhi.hpp"

#include <glm/ext/matrix_float4x4.hpp>

namespace mill
{
    static const std::vector<u32> g_SceneShaderSpirvVS = {
        0x07230203, 0x00010000, 0x0008000b, 0x00000035, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e,
        0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000b000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000d,
        0x00000021, 0x0000002d, 0x0000002f, 0x00000032, 0x00000033, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
        0x00000000, 0x00060005, 0x0000000b, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000000b, 0x00000000, 0x505f6c67,
        0x7469736f, 0x006e6f69, 0x00070006, 0x0000000b, 0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b,
        0x00000002, 0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e, 0x00070006, 0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369,
        0x0065636e, 0x00030005, 0x0000000d, 0x00000000, 0x00050005, 0x00000011, 0x68737550, 0x636f6c42, 0x0000006b, 0x00050006, 0x00000011,
        0x00000000, 0x6a6f7270, 0x00000000, 0x00050006, 0x00000011, 0x00000001, 0x77656976, 0x00000000, 0x00050006, 0x00000011, 0x00000002,
        0x6c726f77, 0x00000064, 0x00050005, 0x00000013, 0x6f635f75, 0x7374736e, 0x00000000, 0x00050005, 0x00000021, 0x705f6e69, 0x7469736f,
        0x006e6f69, 0x00060005, 0x0000002d, 0x5f74756f, 0x43786574, 0x64726f6f, 0x00000000, 0x00050005, 0x0000002f, 0x745f6e69, 0x6f437865,
        0x0064726f, 0x00050005, 0x00000032, 0x5f74756f, 0x6f6c6f63, 0x00000072, 0x00050005, 0x00000033, 0x635f6e69, 0x726f6c6f, 0x00000000,
        0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000, 0x00050048, 0x0000000b, 0x00000001, 0x0000000b, 0x00000001, 0x00050048,
        0x0000000b, 0x00000002, 0x0000000b, 0x00000003, 0x00050048, 0x0000000b, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000000b,
        0x00000002, 0x00040048, 0x00000011, 0x00000000, 0x00000005, 0x00050048, 0x00000011, 0x00000000, 0x00000023, 0x00000000, 0x00050048,
        0x00000011, 0x00000000, 0x00000007, 0x00000010, 0x00040048, 0x00000011, 0x00000001, 0x00000005, 0x00050048, 0x00000011, 0x00000001,
        0x00000023, 0x00000040, 0x00050048, 0x00000011, 0x00000001, 0x00000007, 0x00000010, 0x00040048, 0x00000011, 0x00000002, 0x00000005,
        0x00050048, 0x00000011, 0x00000002, 0x00000023, 0x00000080, 0x00050048, 0x00000011, 0x00000002, 0x00000007, 0x00000010, 0x00030047,
        0x00000011, 0x00000002, 0x00040047, 0x00000021, 0x0000001e, 0x00000000, 0x00040047, 0x0000002d, 0x0000001e, 0x00000000, 0x00040047,
        0x0000002f, 0x0000001e, 0x00000001, 0x00040047, 0x00000032, 0x0000001e, 0x00000001, 0x00040047, 0x00000033, 0x0000001e, 0x00000002,
        0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006,
        0x00000004, 0x00040015, 0x00000008, 0x00000020, 0x00000000, 0x0004002b, 0x00000008, 0x00000009, 0x00000001, 0x0004001c, 0x0000000a,
        0x00000006, 0x00000009, 0x0006001e, 0x0000000b, 0x00000007, 0x00000006, 0x0000000a, 0x0000000a, 0x00040020, 0x0000000c, 0x00000003,
        0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000003, 0x00040015, 0x0000000e, 0x00000020, 0x00000001, 0x0004002b, 0x0000000e,
        0x0000000f, 0x00000000, 0x00040018, 0x00000010, 0x00000007, 0x00000004, 0x0005001e, 0x00000011, 0x00000010, 0x00000010, 0x00000010,
        0x00040020, 0x00000012, 0x00000009, 0x00000011, 0x0004003b, 0x00000012, 0x00000013, 0x00000009, 0x00040020, 0x00000014, 0x00000009,
        0x00000010, 0x0004002b, 0x0000000e, 0x00000017, 0x00000001, 0x0004002b, 0x0000000e, 0x0000001b, 0x00000002, 0x00040017, 0x0000001f,
        0x00000006, 0x00000003, 0x00040020, 0x00000020, 0x00000001, 0x0000001f, 0x0004003b, 0x00000020, 0x00000021, 0x00000001, 0x0004002b,
        0x00000006, 0x00000023, 0x3f800000, 0x00040020, 0x00000029, 0x00000003, 0x00000007, 0x00040017, 0x0000002b, 0x00000006, 0x00000002,
        0x00040020, 0x0000002c, 0x00000003, 0x0000002b, 0x0004003b, 0x0000002c, 0x0000002d, 0x00000003, 0x00040020, 0x0000002e, 0x00000001,
        0x0000002b, 0x0004003b, 0x0000002e, 0x0000002f, 0x00000001, 0x00040020, 0x00000031, 0x00000003, 0x0000001f, 0x0004003b, 0x00000031,
        0x00000032, 0x00000003, 0x0004003b, 0x00000020, 0x00000033, 0x00000001, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003,
        0x000200f8, 0x00000005, 0x00050041, 0x00000014, 0x00000015, 0x00000013, 0x0000000f, 0x0004003d, 0x00000010, 0x00000016, 0x00000015,
        0x00050041, 0x00000014, 0x00000018, 0x00000013, 0x00000017, 0x0004003d, 0x00000010, 0x00000019, 0x00000018, 0x00050092, 0x00000010,
        0x0000001a, 0x00000016, 0x00000019, 0x00050041, 0x00000014, 0x0000001c, 0x00000013, 0x0000001b, 0x0004003d, 0x00000010, 0x0000001d,
        0x0000001c, 0x00050092, 0x00000010, 0x0000001e, 0x0000001a, 0x0000001d, 0x0004003d, 0x0000001f, 0x00000022, 0x00000021, 0x00050051,
        0x00000006, 0x00000024, 0x00000022, 0x00000000, 0x00050051, 0x00000006, 0x00000025, 0x00000022, 0x00000001, 0x00050051, 0x00000006,
        0x00000026, 0x00000022, 0x00000002, 0x00070050, 0x00000007, 0x00000027, 0x00000024, 0x00000025, 0x00000026, 0x00000023, 0x00050091,
        0x00000007, 0x00000028, 0x0000001e, 0x00000027, 0x00050041, 0x00000029, 0x0000002a, 0x0000000d, 0x0000000f, 0x0003003e, 0x0000002a,
        0x00000028, 0x0004003d, 0x0000002b, 0x00000030, 0x0000002f, 0x0003003e, 0x0000002d, 0x00000030, 0x0004003d, 0x0000001f, 0x00000034,
        0x00000033, 0x0003003e, 0x00000032, 0x00000034, 0x000100fd, 0x00010038
    };

    static const std::vector<u32> g_SceneShaderSpirvFS = {
        0x07230203, 0x00010000, 0x0008000b, 0x00000016, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e,
        0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0008000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009,
        0x0000000c, 0x00000015, 0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
        0x00000000, 0x00060005, 0x00000009, 0x5f74756f, 0x67617266, 0x6f6c6f43, 0x00000072, 0x00050005, 0x0000000c, 0x635f6e69, 0x726f6c6f,
        0x00000000, 0x00050005, 0x00000015, 0x745f6e69, 0x6f437865, 0x0064726f, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047,
        0x0000000c, 0x0000001e, 0x00000001, 0x00040047, 0x00000015, 0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
        0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003,
        0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040017, 0x0000000a, 0x00000006, 0x00000003, 0x00040020, 0x0000000b,
        0x00000001, 0x0000000a, 0x0004003b, 0x0000000b, 0x0000000c, 0x00000001, 0x0004002b, 0x00000006, 0x0000000e, 0x3f800000, 0x00040017,
        0x00000013, 0x00000006, 0x00000002, 0x00040020, 0x00000014, 0x00000001, 0x00000013, 0x0004003b, 0x00000014, 0x00000015, 0x00000001,
        0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x0000000a, 0x0000000d, 0x0000000c,
        0x00050051, 0x00000006, 0x0000000f, 0x0000000d, 0x00000000, 0x00050051, 0x00000006, 0x00000010, 0x0000000d, 0x00000001, 0x00050051,
        0x00000006, 0x00000011, 0x0000000d, 0x00000002, 0x00070050, 0x00000007, 0x00000012, 0x0000000f, 0x00000010, 0x00000011, 0x0000000e,
        0x0003003e, 0x00000009, 0x00000012, 0x000100fd, 0x00010038
    };

    struct SceneRenderInstance
    {
        glm::mat4 worldMat{ 1.0f };
    };

    struct SceneRenderInfo
    {
        glm::mat4 cameraProjMat{ 1.0f };
        glm::mat4 cameraViewMat{ 1.0f };

        std::vector<SceneRenderInstance> renderInstances{};
    };

    class SceneRenderer
    {
    public:
        SceneRenderer(u64 view_id) : m_viewId(view_id) {}
        ~SceneRenderer() = default;

        void initialise()
        {
            rhi::reset_view(m_viewId, 1600, 900);

            // Pipeline
            {
                rhi::PipelineDescription pipeline_desc{
                .vertexInputState = { 
                    .attributes = {
                        { "Position", rhi::Format::eRGB32 },
                        { "TexCoord", rhi::Format::eRG32 },
                        { "Color", rhi::Format::eRGB32 },
                    },
                    .topology = rhi::PrimitiveTopology::eTriangles,
                    },
                    .preRasterisationState = {
                        .vertexSpirv = g_SceneShaderSpirvVS,
                        .fillMode = rhi::FillMode::eFill,
                        .cullMode = rhi::CullMode::eNone,
                        .frontFace = rhi::FrontFace::eClockwise,
                        .lineWidth = 1.0f,
                    },
                    .fragmentStageState = {
                        .fragmentSpirv = g_SceneShaderSpirvFS,
                        .depthTest = false,
                        .stencilTest = false,
                    },
                    .fragmentOutputState = {
                        .enableColorBlend = false,
                    },
                };

                m_pipeline = rhi::create_pipeline(pipeline_desc);
            }

            // Triangle Vertex Buffer
            {
                const std::vector<StaticVertex> triangleVertices = {
                    { { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
                    { { -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
                    { { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
                };

                rhi::BufferDescription buffer_desc{};
                buffer_desc.size = sizeof(StaticVertex) * triangleVertices.size();
                buffer_desc.usage = rhi::BufferUsage::eVertexBuffer;
                buffer_desc.memoryUsage = rhi::MemoryUsage::eDeviceHostVisble;
                m_triangleVertexBuffer = rhi::create_buffer(buffer_desc);

                rhi::write_buffer(m_triangleVertexBuffer, 0, buffer_desc.size, triangleVertices.data());
            }
            // Triangle Index Buffer
            {
                const std::vector<u16> triangleIndices = { 0, 1, 2 };

                rhi::BufferDescription buffer_desc{};
                buffer_desc.size = sizeof(u16) * triangleIndices.size();
                buffer_desc.usage = rhi::BufferUsage::eIndexBuffer;
                buffer_desc.memoryUsage = rhi::MemoryUsage::eDeviceHostVisble;
                m_triangleIndexBuffer = rhi::create_buffer(buffer_desc);

                rhi::write_buffer(m_triangleIndexBuffer, 0, buffer_desc.size, triangleIndices.data());
            }
        }

        auto render(u64 context, const SceneRenderInfo& scene_info) -> u64
        {
            rhi::begin_view(context, m_viewId, { 0.392f, 0.584f, 0.929f, 1 });
            {
                rhi::set_viewport(context, 0, 0, 1600, 900, 0.0f, 1.0f);
                rhi::set_scissor(context, 0, 0, 1600, 900);

                rhi::set_pipeline(context, m_pipeline);

                rhi::set_push_constants(context, 0, sizeof(glm::mat4), &scene_info.cameraProjMat);
                rhi::set_push_constants(context, sizeof(glm::mat4), sizeof(glm::mat4), &scene_info.cameraViewMat);

                rhi::set_index_buffer(context, m_triangleIndexBuffer, rhi::IndexType::eU16);
                rhi::set_vertex_buffer(context, m_triangleVertexBuffer);

                for (const auto& instance : scene_info.renderInstances)
                {
                    rhi::set_push_constants(context, sizeof(glm::mat4) * 2, sizeof(glm::mat4), &instance.worldMat);
                    rhi::draw_indexed(context, 3);
                }
            }
            rhi::end_view(context, m_viewId);

            return m_viewId;
        }

    private:
        u64 m_viewId{};

        u64 m_pipeline{};
        rhi::HandleBuffer m_triangleIndexBuffer{};
        rhi::HandleBuffer m_triangleVertexBuffer{};
    };
}