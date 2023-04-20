#include "renderer.hpp"

#include <mill/mill.hpp>

#include <imgui.h>

// glsl_shader.vert, compiled with:
// # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
/*
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;
out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;
void main()
{
    Out.Color = aColor;
    Out.UV = aUV;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
*/
static std::vector<mill::u32> __glsl_shader_vert_spv = {
    0x07230203, 0x00010000, 0x00080001, 0x0000002e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e,
    0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000a000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000b,
    0x0000000f, 0x00000015, 0x0000001b, 0x0000001c, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000,
    0x00030005, 0x00000009, 0x00000000, 0x00050006, 0x00000009, 0x00000000, 0x6f6c6f43, 0x00000072, 0x00040006, 0x00000009, 0x00000001,
    0x00005655, 0x00030005, 0x0000000b, 0x0074754f, 0x00040005, 0x0000000f, 0x6c6f4361, 0x0000726f, 0x00030005, 0x00000015, 0x00565561,
    0x00060005, 0x00000019, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x00000019, 0x00000000, 0x505f6c67, 0x7469736f,
    0x006e6f69, 0x00030005, 0x0000001b, 0x00000000, 0x00040005, 0x0000001c, 0x736f5061, 0x00000000, 0x00060005, 0x0000001e, 0x73755075,
    0x6e6f4368, 0x6e617473, 0x00000074, 0x00050006, 0x0000001e, 0x00000000, 0x61635375, 0x0000656c, 0x00060006, 0x0000001e, 0x00000001,
    0x61725475, 0x616c736e, 0x00006574, 0x00030005, 0x00000020, 0x00006370, 0x00040047, 0x0000000b, 0x0000001e, 0x00000000, 0x00040047,
    0x0000000f, 0x0000001e, 0x00000002, 0x00040047, 0x00000015, 0x0000001e, 0x00000001, 0x00050048, 0x00000019, 0x00000000, 0x0000000b,
    0x00000000, 0x00030047, 0x00000019, 0x00000002, 0x00040047, 0x0000001c, 0x0000001e, 0x00000000, 0x00050048, 0x0000001e, 0x00000000,
    0x00000023, 0x00000000, 0x00050048, 0x0000001e, 0x00000001, 0x00000023, 0x00000008, 0x00030047, 0x0000001e, 0x00000002, 0x00020013,
    0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004,
    0x00040017, 0x00000008, 0x00000006, 0x00000002, 0x0004001e, 0x00000009, 0x00000007, 0x00000008, 0x00040020, 0x0000000a, 0x00000003,
    0x00000009, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000003, 0x00040015, 0x0000000c, 0x00000020, 0x00000001, 0x0004002b, 0x0000000c,
    0x0000000d, 0x00000000, 0x00040020, 0x0000000e, 0x00000001, 0x00000007, 0x0004003b, 0x0000000e, 0x0000000f, 0x00000001, 0x00040020,
    0x00000011, 0x00000003, 0x00000007, 0x0004002b, 0x0000000c, 0x00000013, 0x00000001, 0x00040020, 0x00000014, 0x00000001, 0x00000008,
    0x0004003b, 0x00000014, 0x00000015, 0x00000001, 0x00040020, 0x00000017, 0x00000003, 0x00000008, 0x0003001e, 0x00000019, 0x00000007,
    0x00040020, 0x0000001a, 0x00000003, 0x00000019, 0x0004003b, 0x0000001a, 0x0000001b, 0x00000003, 0x0004003b, 0x00000014, 0x0000001c,
    0x00000001, 0x0004001e, 0x0000001e, 0x00000008, 0x00000008, 0x00040020, 0x0000001f, 0x00000009, 0x0000001e, 0x0004003b, 0x0000001f,
    0x00000020, 0x00000009, 0x00040020, 0x00000021, 0x00000009, 0x00000008, 0x0004002b, 0x00000006, 0x00000028, 0x00000000, 0x0004002b,
    0x00000006, 0x00000029, 0x3f800000, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d,
    0x00000007, 0x00000010, 0x0000000f, 0x00050041, 0x00000011, 0x00000012, 0x0000000b, 0x0000000d, 0x0003003e, 0x00000012, 0x00000010,
    0x0004003d, 0x00000008, 0x00000016, 0x00000015, 0x00050041, 0x00000017, 0x00000018, 0x0000000b, 0x00000013, 0x0003003e, 0x00000018,
    0x00000016, 0x0004003d, 0x00000008, 0x0000001d, 0x0000001c, 0x00050041, 0x00000021, 0x00000022, 0x00000020, 0x0000000d, 0x0004003d,
    0x00000008, 0x00000023, 0x00000022, 0x00050085, 0x00000008, 0x00000024, 0x0000001d, 0x00000023, 0x00050041, 0x00000021, 0x00000025,
    0x00000020, 0x00000013, 0x0004003d, 0x00000008, 0x00000026, 0x00000025, 0x00050081, 0x00000008, 0x00000027, 0x00000024, 0x00000026,
    0x00050051, 0x00000006, 0x0000002a, 0x00000027, 0x00000000, 0x00050051, 0x00000006, 0x0000002b, 0x00000027, 0x00000001, 0x00070050,
    0x00000007, 0x0000002c, 0x0000002a, 0x0000002b, 0x00000028, 0x00000029, 0x00050041, 0x00000011, 0x0000002d, 0x0000001b, 0x0000000d,
    0x0003003e, 0x0000002d, 0x0000002c, 0x000100fd, 0x00010038
};

// glsl_shader.frag, compiled with:
// # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag
/*
#version 450 core
layout(location = 0) out vec4 fColor;
layout(set=0, binding=0) uniform sampler2D sTexture;
layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
void main()
{
    fColor = In.Color * texture(sTexture, In.UV.st);
}
*/
static std::vector<mill::u32> __glsl_shader_frag_spv = {
    0x07230203, 0x00010000, 0x00080001, 0x0000001e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e,
    0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009,
    0x0000000d, 0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000,
    0x00040005, 0x00000009, 0x6c6f4366, 0x0000726f, 0x00030005, 0x0000000b, 0x00000000, 0x00050006, 0x0000000b, 0x00000000, 0x6f6c6f43,
    0x00000072, 0x00040006, 0x0000000b, 0x00000001, 0x00005655, 0x00030005, 0x0000000d, 0x00006e49, 0x00050005, 0x00000016, 0x78655473,
    0x65727574, 0x00000000, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000d, 0x0000001e, 0x00000000, 0x00040047,
    0x00000016, 0x00000022, 0x00000000, 0x00040047, 0x00000016, 0x00000021, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
    0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003,
    0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040017, 0x0000000a, 0x00000006, 0x00000002, 0x0004001e, 0x0000000b,
    0x00000007, 0x0000000a, 0x00040020, 0x0000000c, 0x00000001, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000001, 0x00040015,
    0x0000000e, 0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000, 0x00040020, 0x00000010, 0x00000001, 0x00000007,
    0x00090019, 0x00000013, 0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x0003001b, 0x00000014,
    0x00000013, 0x00040020, 0x00000015, 0x00000000, 0x00000014, 0x0004003b, 0x00000015, 0x00000016, 0x00000000, 0x0004002b, 0x0000000e,
    0x00000018, 0x00000001, 0x00040020, 0x00000019, 0x00000001, 0x0000000a, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003,
    0x000200f8, 0x00000005, 0x00050041, 0x00000010, 0x00000011, 0x0000000d, 0x0000000f, 0x0004003d, 0x00000007, 0x00000012, 0x00000011,
    0x0004003d, 0x00000014, 0x00000017, 0x00000016, 0x00050041, 0x00000019, 0x0000001a, 0x0000000d, 0x00000018, 0x0004003d, 0x0000000a,
    0x0000001b, 0x0000001a, 0x00050057, 0x00000007, 0x0000001c, 0x00000017, 0x0000001b, 0x00050085, 0x00000007, 0x0000001d, 0x00000012,
    0x0000001c, 0x0003003e, 0x00000009, 0x0000001d, 0x000100fd, 0x00010038
};

namespace mill::asset_browser
{
    Renderer::Renderer(u64 view_id) : m_viewId(view_id) {}

    void Renderer::initialise()
    {
        auto& io = ImGui::GetIO();
        io.BackendFlags |=
            ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

        // ImGui Font Texture
        {
            int width, height;
            unsigned char* pixels = nullptr;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

            rhi::TextureDescription texture_desc{
                .dimensions = { width, height, 1 },
                .format = rhi::Format::eRGBA8,
                .mipLevels = 1,
            };
            m_fontTexture = rhi::create_texture(texture_desc);
            rhi::write_texture(m_fontTexture, 0, pixels);
        }
        // ImGui Font Resource Set
        {
            rhi::ResourceSetDescription set_desc{
                .bindings = { 
                    {rhi::ResourceType::eTexture, 1, rhi::ShaderStage::eFragment},    
                },
            };
            m_fontResourceSet = rhi::create_resource_set(set_desc);
            rhi::bind_texture_to_resource_set(m_fontResourceSet, 0, m_fontTexture);

            io.Fonts->SetTexID(&m_fontResourceSet);
        }

        // ImGui Pipeline
        {
            rhi::PipelineDescription pipeline_desc{
                .vertexInputState = { 
                    .attributes = {
                        { "pos", rhi::Format::eRG32 },
                        { "uv", rhi::Format::eRG32 },
                        { "color", rhi::Format::eRGBA8 },
                    },
                    .topology = rhi::PrimitiveTopology::eTriangles,
                    },
                    .preRasterisationState = {
                        .vertexSpirv = __glsl_shader_vert_spv,
                        .fillMode = rhi::FillMode::eFill,
                        .cullMode = rhi::CullMode::eNone,
                        .frontFace = rhi::FrontFace::eClockwise,
                        .lineWidth = 1.0f,
                    },
                    .fragmentStageState = {
                        .fragmentSpirv = __glsl_shader_frag_spv,
                        .depthTest = false,
                        .stencilTest = false,
                    },
                    .fragmentOutputState = {
                        .enableColorBlend = true,
                    },
                };
            m_imguiPipeline = rhi::create_pipeline(pipeline_desc);
        }

        rhi::reset_view(m_viewId, 1600, 900);
    }

    void Renderer::shutdown() {}

    auto Renderer::render(u64 context_id) -> u64
    {
        ImGui::Render();

        rhi::begin_view(context_id, m_viewId, { 0, 0, 0, 1 });
        {
            const auto* draw_data = ImGui::GetDrawData();
            render_draw_data(context_id, draw_data);
        }
        rhi::end_view(context_id, m_viewId);

        return m_viewId;
    }

    void Renderer::render_draw_data(u64 context_id, const ImDrawData* draw_data)
    {
        // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
        const f32 fb_width = draw_data->DisplaySize.x * draw_data->FramebufferScale.x;
        const f32 fb_height = draw_data->DisplaySize.y * draw_data->FramebufferScale.y;
        if (fb_width <= 0 || fb_height <= 0)
            return;

        m_bufferIndex = (m_bufferIndex + 1) % m_buffers.size();
        auto& buffer = m_buffers[m_bufferIndex];

        if (draw_data->TotalVtxCount > 0)
        {
            sizet vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
            if (!buffer.vertexBuffer || buffer.vertexSize < vertex_size)
            {
                if (buffer.vertexBuffer)
                    rhi::destroy_buffer(buffer.vertexBuffer);

                rhi::BufferDescription buffer_desc{
                    .size = vertex_size,
                    .usage = rhi::BufferUsage::eVertexBuffer,
                    .memoryUsage = rhi::MemoryUsage::eDeviceHostVisble,
                };
                buffer.vertexBuffer = rhi::create_buffer(buffer_desc);
                buffer.vertexSize = vertex_size;
            }
            sizet index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
            if (!buffer.indexBuffer || buffer.indexSize < index_size)
            {
                if (buffer.indexBuffer)
                    rhi::destroy_buffer(buffer.indexBuffer);

                rhi::BufferDescription buffer_desc{
                    .size = index_size,
                    .usage = rhi::BufferUsage::eIndexBuffer,
                    .memoryUsage = rhi::MemoryUsage::eDeviceHostVisble,
                };
                buffer.indexBuffer = rhi::create_buffer(buffer_desc);
                buffer.indexSize = index_size;
            }

            // Write buffers
            u64 vtx_offset{};
            u64 idx_offset{};
            for (i32 i = 0; i < draw_data->CmdListsCount; ++i)
            {
                const auto* cmd_list = draw_data->CmdLists[i];

                const auto vtx_data_size = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
                rhi::write_buffer(buffer.vertexBuffer, vtx_offset, vtx_data_size, cmd_list->VtxBuffer.Data);
                vtx_offset += vtx_data_size;

                const auto idx_data_size = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
                rhi::write_buffer(buffer.indexBuffer, idx_offset, idx_data_size, cmd_list->IdxBuffer.Data);
                idx_offset += idx_data_size;
            }
        }

        rhi::set_pipeline(context_id, m_imguiPipeline);

        if (draw_data->TotalVtxCount > 0)
        {
            rhi::set_vertex_buffer(context_id, buffer.vertexBuffer);
            rhi::set_index_buffer(context_id, buffer.indexBuffer, rhi::IndexType::eU16);
        }

        rhi::set_viewport(context_id,
                          draw_data->DisplayPos.x,
                          draw_data->DisplayPos.y + draw_data->DisplaySize.y,
                          draw_data->DisplayPos.x + draw_data->DisplaySize.x,
                          -(draw_data->DisplayPos.y + draw_data->DisplaySize.y));

        f32 scale[2];
        scale[0] = 2.0f / draw_data->DisplaySize.x;
        scale[1] = 2.0f / draw_data->DisplaySize.y;
        f32 translate[2];
        translate[0] = -1.0f - draw_data->DisplayPos.x * scale[0];
        translate[1] = -1.0f - draw_data->DisplayPos.y * scale[1];
        rhi::set_push_constants(context_id, sizeof(f32) * 0, sizeof(f32) * 2, scale);
        rhi::set_push_constants(context_id, sizeof(f32) * 2, sizeof(f32) * 2, translate);

        ImVec2 clip_offset = draw_data->DisplayPos;

        u32 global_index_offset{};
        u32 global_vertex_offset{};
        for (i32 i = 0; i < draw_data->CmdListsCount; ++i)
        {
            const auto* cmd_list = draw_data->CmdLists[i];
            // const auto* vtx_buffer = cmd_list->VtxBuffer.Data;  // Vertex buffer generated by ImGui
            // const auto* idx_buffer = cmd_list->IdxBuffer.Data;  // Index buffer generated by ImGui
            for (i32 cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
            {
                const auto* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback)
                {
                    pcmd->UserCallback(cmd_list, pcmd);
                }
                else
                {
                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec2 clip_min(pcmd->ClipRect.x - clip_offset.x, pcmd->ClipRect.y - clip_offset.y);
                    ImVec2 clip_max(pcmd->ClipRect.z - clip_offset.x, pcmd->ClipRect.w - clip_offset.y);

                    // Clamp clip to viewport
                    clip_min.x = std::max(0.0f, clip_min.x);
                    clip_min.y = std::max(0.0f, clip_min.y);
                    clip_max.x = std::min(clip_max.x, fb_width);
                    clip_max.y = std::max(clip_max.y, fb_height);
                    if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                        continue;

                    rhi::set_scissor(context_id,
                                     CAST_I32(clip_min.x),
                                     CAST_I32(clip_min.y),
                                     CAST_U32(clip_max.x - clip_min.x),
                                     CAST_U32(clip_max.y - clip_min.y));

                    rhi::set_resource_sets(context_id, { *(u64*)pcmd->GetTexID() });

                    rhi::draw_indexed(
                        context_id, pcmd->ElemCount, 1, pcmd->IdxOffset + global_index_offset, pcmd->VtxOffset + global_vertex_offset);
                }
            }

            global_index_offset += cmd_list->IdxBuffer.Size;
            global_vertex_offset += cmd_list->VtxBuffer.Size;
        }
    }
}