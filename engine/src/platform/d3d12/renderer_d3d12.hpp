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
#if 0
        static const u32 FrameCount = 2;
        //IDXGIFactory7* m_factory = nullptr;
        //ID3D12Device10* m_device = nullptr;
        DWORD m_debugCallbackCookie{};
        // Owned<CommandD3D12> m_cmd = nullptr;

        ID3D12CommandQueue* m_cmdQueue = nullptr;
        ID3D12CommandAllocator* m_cmdAllocator = nullptr;
        ID3D12GraphicsCommandList* m_cmdList = nullptr;

        ID3D12Fence1* m_fence = nullptr;
        u64 m_fenceValue = 0;
        HANDLE m_fenceEvent = nullptr;

        ID3D12DescriptorHeap* m_rtvHeap = nullptr;
        u32 m_rtvDescriptorSize = 0;

        ComPtr<IDXGISwapChain4> m_swapchain = nullptr;
        ID3D12Resource* m_renderTargets[FrameCount];
        u32 m_backbufferIndex = 0;
#endif
    };
}