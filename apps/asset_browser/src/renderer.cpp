#include "renderer.hpp"

namespace mill::asset_browser
{
    Renderer::Renderer(u64 view_id) : m_viewId(view_id) {}

    void Renderer::initialise()
    {
        rhi::reset_view(m_viewId, 1600, 900);
    }

    void Renderer::shutdown() {}

    auto Renderer::render(u64 context_id) -> u64
    {
        rhi::begin_view(context_id, m_viewId, { 0, 0, 0, 1 });
        {
        }
        rhi::end_view(context_id, m_viewId);

        return m_viewId;
    }

}