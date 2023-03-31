#include <mill/entry_point.hpp>
#include <mill/mill.hpp>

using namespace mill;

static const std::vector<u32> g_TriangleShaderSpirvVS = {
    0x07230203, 0x00010000, 0x0008000b, 0x0000001f, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e,
    0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0009000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000d,
    0x00000012, 0x0000001c, 0x0000001d, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00060005,
    0x0000000b, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000000b, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69,
    0x00070006, 0x0000000b, 0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b, 0x00000002, 0x435f6c67,
    0x4470696c, 0x61747369, 0x0065636e, 0x00070006, 0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369, 0x0065636e, 0x00030005,
    0x0000000d, 0x00000000, 0x00050005, 0x00000012, 0x705f6e69, 0x7469736f, 0x006e6f69, 0x00050005, 0x0000001c, 0x5f74756f, 0x6f6c6f63,
    0x00000072, 0x00050005, 0x0000001d, 0x635f6e69, 0x726f6c6f, 0x00000000, 0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000,
    0x00050048, 0x0000000b, 0x00000001, 0x0000000b, 0x00000001, 0x00050048, 0x0000000b, 0x00000002, 0x0000000b, 0x00000003, 0x00050048,
    0x0000000b, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000000b, 0x00000002, 0x00040047, 0x00000012, 0x0000001e, 0x00000000,
    0x00040047, 0x0000001c, 0x0000001e, 0x00000000, 0x00040047, 0x0000001d, 0x0000001e, 0x00000001, 0x00020013, 0x00000002, 0x00030021,
    0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040015, 0x00000008,
    0x00000020, 0x00000000, 0x0004002b, 0x00000008, 0x00000009, 0x00000001, 0x0004001c, 0x0000000a, 0x00000006, 0x00000009, 0x0006001e,
    0x0000000b, 0x00000007, 0x00000006, 0x0000000a, 0x0000000a, 0x00040020, 0x0000000c, 0x00000003, 0x0000000b, 0x0004003b, 0x0000000c,
    0x0000000d, 0x00000003, 0x00040015, 0x0000000e, 0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000, 0x00040017,
    0x00000010, 0x00000006, 0x00000003, 0x00040020, 0x00000011, 0x00000001, 0x00000010, 0x0004003b, 0x00000011, 0x00000012, 0x00000001,
    0x0004002b, 0x00000006, 0x00000014, 0x3f800000, 0x00040020, 0x00000019, 0x00000003, 0x00000007, 0x00040020, 0x0000001b, 0x00000003,
    0x00000010, 0x0004003b, 0x0000001b, 0x0000001c, 0x00000003, 0x0004003b, 0x00000011, 0x0000001d, 0x00000001, 0x00050036, 0x00000002,
    0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000010, 0x00000013, 0x00000012, 0x00050051, 0x00000006,
    0x00000015, 0x00000013, 0x00000000, 0x00050051, 0x00000006, 0x00000016, 0x00000013, 0x00000001, 0x00050051, 0x00000006, 0x00000017,
    0x00000013, 0x00000002, 0x00070050, 0x00000007, 0x00000018, 0x00000015, 0x00000016, 0x00000017, 0x00000014, 0x00050041, 0x00000019,
    0x0000001a, 0x0000000d, 0x0000000f, 0x0003003e, 0x0000001a, 0x00000018, 0x0004003d, 0x00000010, 0x0000001e, 0x0000001d, 0x0003003e,
    0x0000001c, 0x0000001e, 0x000100fd, 0x00010038
};

static const std::vector<u32> g_TriangleShaderSpirvFS = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000013, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e,
    0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009,
    0x0000000c, 0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000,
    0x00060005, 0x00000009, 0x5f74756f, 0x67617266, 0x6f6c6f43, 0x00000072, 0x00050005, 0x0000000c, 0x635f6e69, 0x726f6c6f, 0x00000000,
    0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000c, 0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021,
    0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008,
    0x00000003, 0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040017, 0x0000000a, 0x00000006, 0x00000003, 0x00040020,
    0x0000000b, 0x00000001, 0x0000000a, 0x0004003b, 0x0000000b, 0x0000000c, 0x00000001, 0x0004002b, 0x00000006, 0x0000000e, 0x3f800000,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x0000000a, 0x0000000d, 0x0000000c,
    0x00050051, 0x00000006, 0x0000000f, 0x0000000d, 0x00000000, 0x00050051, 0x00000006, 0x00000010, 0x0000000d, 0x00000001, 0x00050051,
    0x00000006, 0x00000011, 0x0000000d, 0x00000002, 0x00070050, 0x00000007, 0x00000012, 0x0000000f, 0x00000010, 0x00000011, 0x0000000e,
    0x0003003e, 0x00000009, 0x00000012, 0x000100fd, 0x00010038
};

struct Vertex
{
    glm::vec3 position{};
    glm::vec3 color{};
};

static const std::vector<Vertex> g_TriangleVertices = {
    { { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
};

class SandboxApp : public mill::Application
{
public:
    const u64 SceneViewId = "scene_view"_hs;

    void initialise() override
    {
        auto& events = Engine::get()->get_events();
        events.subscribe([this](const Event& event) { event_callback(event); });

        m_sceneRenderer = CreateOwned<SceneRenderer>(SceneViewId);

        platform::WindowInfo window_info{
            .title = "Sandbox",
            .width = 1600,
            .height = 900,
        };
        m_windowHandle = platform::create_window(window_info);

        rhi::initialise();

        rhi::assign_screen(0, platform::get_raw_window_handle(m_windowHandle));
        rhi::reset_screen(0, 1600, 900, true);
        rhi::reset_view(SceneViewId, 1600, 900);

        // Triangle Pipeline
        {
            rhi::PipelineDescription pipeline_desc{};
            pipeline_desc.vs = g_TriangleShaderSpirvVS;
            pipeline_desc.fs = g_TriangleShaderSpirvFS;
            pipeline_desc.vertexAttributes = {
                { "Position", rhi::Format::eRGB32 },
                { "Color", rhi::Format::eRGB32 },
            };
            pipeline_desc.colorTargets = { rhi::Format::eRGBA8 };
            pipeline_desc.depthStencilTarget = rhi::Format::eD24S8;

            m_trianglePipeline = rhi::create_pipeline(pipeline_desc);
        }
        // Triangle Vertex Buffer
        {
            rhi::BufferDescription buffer_desc{};
            buffer_desc.size = sizeof(Vertex) * g_TriangleVertices.size();
            buffer_desc.usage = rhi::BufferUsage::eVertexBuffer;
            buffer_desc.memoryUsage = rhi::MemoryUsage::eDeviceHostVisble;
            m_triangleVertexBuffer = rhi::create_buffer(buffer_desc);

            rhi::write_buffer(m_triangleVertexBuffer, 0, buffer_desc.size, g_TriangleVertices.data());
        }
    }
    void shutdown() override
    {
        rhi::shutdown();
        platform::destroy_window(m_windowHandle);
    }

    void update() override
    {
        // Update window size. Call when window is resized.
        // rhi::reset_screen(0, 1600, 900, true);

        rhi::begin_frame();
        {
            const static auto RenderContextId = "render_context"_hs;
            rhi::begin_contex(RenderContextId);

            rhi::begin_view(RenderContextId, SceneViewId, { 0.392f, 0.584f, 0.929f, 1 });
            {
                rhi::set_viewport(RenderContextId, 0, 0, 1600, 900, 0.0f, 1.0f);
                rhi::set_scissor(RenderContextId, 0, 0, 1600, 900);
                rhi::set_pipeline(RenderContextId, m_trianglePipeline);
                rhi::set_vertex_buffer(RenderContextId, m_triangleVertexBuffer);
                rhi::draw(RenderContextId, 3);
            }
            rhi::end_view(RenderContextId, SceneViewId);

            rhi::blit_to_screen(RenderContextId, 0, SceneViewId);

            rhi::end_context(RenderContextId);
        }
        rhi::end_frame();
    }

private:
    void event_callback(const Event& event)
    {
        if (event.type == EventType::eWindowClose)
        {
            const platform::HandleWindow windowContext = event.context;
            if (windowContext == m_windowHandle)
            {
                Engine::get()->quit();
            }
        }
        else if (event.type == EventType::eWindowSize)
        {
            const platform::HandleWindow windowContext = event.context;
            if (windowContext == m_windowHandle)
            {
                u32 width = event.data.u32[0];
                u32 height = event.data.u32[1];
                rhi::reset_screen(0, width, height, true);
                rhi::reset_view(SceneViewId, width, height);
            }
        }
    }

private:
    platform::HandleWindow m_windowHandle{ nullptr };
    Owned<SceneRenderer> m_sceneRenderer{ nullptr };

    rhi::HandlePipeline m_trianglePipeline{};
    rhi::HandleBuffer m_triangleVertexBuffer{};
};

auto mill::create_application() -> mill::Application*
{
    return new SandboxApp;
}