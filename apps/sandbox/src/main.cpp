#include <mill/entry_point.hpp>
#include <mill/mill.hpp>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

using namespace mill;

class SandboxApp : public Application
{
public:
    const u64 SceneViewId = "scene_view"_hs;

    void initialise() override
    {
        auto& events = Engine::get()->get_events();
        events.subscribe([this](const Event& event) { event_callback(event); });

        platform::WindowInfo window_info{
            .title = "Sandbox",
            .pos_x = 10,
            .pos_y = 40,
            .width = 1600,
            .height = 900,
        };
        m_windowHandle = platform::create_window(window_info);

        rhi::assign_screen(0, platform::get_raw_window_handle(m_windowHandle));
        rhi::reset_screen(0, 1600, 900, true);

#if 0
        // Triangle Resource Set
        {
            rhi::ResourceSetDescription set_desc{ 
                .bindings = {
                    {
                        rhi::ResourceType::eUniformBuffer,
                        1,
                        rhi::ShaderStage::eVertex,
                    },
                },
                .isBuffered = false,
            };
            rhi::create_resource_set(m_triangleResourceSet, set_desc);
        }
#endif

        // Init push constants
        {
            m_cameraProjMat = glm::perspectiveLH_ZO(glm::radians(70.0f), 1600.0f / 900.0f, 0.1f, 100.0f);

            const glm::vec3 eye{ -3, 4, -6 };
            const glm::vec3 target{ 0, 2, 0 };
            m_cameraViewMat = glm::lookAtLH(eye, target, glm::vec3(0, 1, 0));
        }

        m_sceneRenderer = CreateOwned<SceneRenderer>(SceneViewId);
        m_sceneRenderer->initialise();
    }

    void shutdown() override
    {
        platform::destroy_window(m_windowHandle);
    }

    void update(f32 delta_time) override
    {
        UNUSED(delta_time);

        auto& scene = Engine::get()->get_scene_manager()->get_active_scene();
        auto& registry = scene.get_registry();

        // Gather scene render info
        SceneRenderInfo scene_render_info{};
        scene_render_info.cameraProjMat = m_cameraProjMat;
        scene_render_info.cameraViewMat = m_cameraViewMat;

        auto view = registry.view<StaticMeshComponent>();
        for (auto entity : view)
        {
            auto& static_mesh_comp = view.get<StaticMeshComponent>(entity);
            if (!static_mesh_comp.staticMesh)
                continue;

            auto* static_mesh = static_mesh_comp.staticMesh.As<StaticMesh>();
            if (static_mesh == nullptr)
                continue;

            auto& render_instance = scene_render_info.renderInstances.emplace_back();
            render_instance.staticMesh = static_mesh;

            if (registry.all_of<TransformComponent>(entity))
            {
                auto& transform = registry.get<TransformComponent>(entity);
                render_instance.worldMat = transform.get_transform();
            }
        }

        // Update window size. Call when window is resized.
        // rhi::reset_screen(0, 1600, 900, true);

        rhi::begin_frame();
        {
            const static auto RenderContextId = "render_context"_hs;
            rhi::begin_context(RenderContextId);

            auto scene_view_id = m_sceneRenderer->render(RenderContextId, scene_render_info);

            rhi::blit_to_screen(RenderContextId, 0, scene_view_id);

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

    glm::mat4 m_cameraProjMat{ 1.0f };
    glm::mat4 m_cameraViewMat{ 1.0f };
};

auto mill::create_application() -> mill::Application*
{
    return new SandboxApp;
}