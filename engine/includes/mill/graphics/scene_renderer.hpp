#pragma once

#include "mill/graphics/rhi.hpp"

namespace mill
{
    class SceneRenderer
    {
    public:
        SceneRenderer(u64 view_id) : m_viewId(view_id) {}
        ~SceneRenderer() = default;

        void initialise()
        {
            rhi::reset_view(m_viewId, 1600, 900);
        }

        auto render(u64 context /*, const SceneInfo& scene*/) -> u64
        {
            rhi::begin_view(context, m_viewId);

            rhi::end_view(context, m_viewId);

            return m_viewId;
        }

    private:
        u64 m_viewId{};
    };
}