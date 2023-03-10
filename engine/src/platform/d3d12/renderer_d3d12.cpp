#include "renderer_d3d12.hpp"

#include "mill/core/debug.hpp"
#include "d3d12_common_headers.hpp"
#include "d3d12_core.hpp"

#include <vector>

namespace mill::platform
{
    void RendererD3D12::inititialise(const RendererInit& init)
    {
        m_device = CreateOwned<DeviceD3D12>();
        if (!m_device->inititialise())
        {
            LOG_ERROR("RendererD3D12 - Failed to initialise device!");
            return;
        }

        m_graphicsContext = m_device->create_context();

        m_hwnd = static_cast<HWND>(init.window_handle);
        if (!m_device->add_surface(m_hwnd, init.window_width, init.window_height))
        {
            LOG_ERROR("RendererD3D12 - Failed to add surface to device!");
            return;
        }
    }

    void RendererD3D12::shutdown()
    {
        m_device->wait_for_idle();

        m_device->destroy_context(std::move(m_graphicsContext));

        if (!m_device->shutdown())
        {
            LOG_ERROR("RendererD3D12 - Failed to shutdown device!");
        }
        m_device = nullptr;
    }

    void RendererD3D12::render(const SceneInfo& /*scene_info*/)
    {
        m_device->begin_frame();

        m_graphicsContext->reset();

        auto& backbuffer = *m_device->get_current_back_buffer(m_hwnd);

        m_graphicsContext->add_barrier(backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

        m_graphicsContext->clear_render_target(backbuffer, { 0.36f, 0.54f, 0.86f, 1.0f });

        m_graphicsContext->add_barrier(backbuffer, D3D12_RESOURCE_STATE_PRESENT);

        m_device->submit_context(*m_graphicsContext);

        m_device->end_frame();
        m_device->present();
    }

    auto create_renderer() -> Owned<RendererInterface>
    {
        return CreateOwned<RendererD3D12>();
    }

}
