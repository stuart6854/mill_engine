#pragma once

#include "mill/graphics/renderer.hpp"
#include "d3d12_common_headers.hpp"
#include "d3d12_device.hpp"
#include "d3d12_context.hpp"

#include <glm/ext/vector_uint2.hpp>

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
        glm::uvec2 m_screenSize{};

        ComPtr<ID3D12RootSignature> m_rootSignature{ nullptr };
        ComPtr<ID3D12PipelineState> m_pipelineState{ nullptr };

        Owned<BufferD3D12> m_vertexBuffer{ nullptr };
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
        Owned<BufferD3D12> m_indexBuffer{ nullptr };
        D3D12_INDEX_BUFFER_VIEW m_indexBufferView{};

        ComPtr<ID3D12DescriptorHeap> m_cbvHeap{ nullptr };
        Owned<BufferD3D12> m_constantBuffer{ nullptr };
    };
}