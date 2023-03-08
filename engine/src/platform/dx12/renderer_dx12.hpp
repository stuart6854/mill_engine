#pragma once

#include "mill/graphics/renderer.hpp"
#include "dx12_common_headers.hpp"

namespace mill::platform
{
    class RendererDX12 : public RendererInterface
    {
    public:
        RendererDX12() = default;
        ~RendererDX12() = default;

        /* Commands */

        void inititialise(const RendererInit& init) override;
        void shutdown() override;

        void render(const SceneInfo& scene_info) override;

    private:
        IDXGIFactory7* m_dxgi_factory = nullptr;
        ID3D12Device10* m_device = nullptr;
    };
}