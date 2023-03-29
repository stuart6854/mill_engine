#include <mill/entry_point.hpp>
#include <mill/mill.hpp>

using namespace mill;

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

            rhi::begin_view(RenderContextId, SceneViewId, { 1, 0, 0, 1 });

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
    }

private:
    platform::HandleWindow m_windowHandle{ nullptr };
};

auto mill::create_application() -> mill::Application*
{
    return new SandboxApp;
}