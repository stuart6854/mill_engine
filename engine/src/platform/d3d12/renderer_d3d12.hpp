#pragma once

#include "mill/graphics/renderer.hpp"
#include "d3d12_common_headers.hpp"
#include "d3d12_device.hpp"
#include "d3d12_context.hpp"

namespace mill::platform
{
    class CommandD3D12;

    class RendererD3D12 : public RendererInterface
    {
    public:
        RendererD3D12() = default;
        ~RendererD3D12() = default;

        /* Commands */

        void inititialise(const RendererInit& init) override;
        void shutdown() override;

        void render(const SceneInfo& scene_info) override;

    private:
        Owned<DeviceD3D12> m_device{ nullptr };
        Owned<ContextD3D12> m_graphicsContext{ nullptr };
        HWND m_hwnd{ nullptr };
    };
}