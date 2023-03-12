#pragma once

#include "mill/core/base.hpp"
#include "d3d12_common_headers.hpp"
#include "d3d12_queue.hpp"
#include "d3d12_resources.hpp"

namespace mill::platform
{
    class ContextD3D12;

    /* Initialises and contains the D3D12 Device. Also responsible for creation/destruction of rendering objects/resources. */
    class DeviceD3D12
    {
    public:
        DeviceD3D12() = default;
        ~DeviceD3D12() = default;

        DISABLE_COPY_AND_MOVE(DeviceD3D12);

        /* Commands */

        bool inititialise();
        bool shutdown();

        void begin_frame();
        void end_frame();
        void present();

        bool add_surface(HWND hwnd, i32 width, i32 height);
        void remove_surface(HWND hwnd);

        auto create_context() -> Owned<ContextD3D12>;

        void destroy_context(Owned<ContextD3D12> context);

        void submit_context(ContextD3D12& context);
        void wait_on_context(/*Some submission receipt*/);
        void wait_for_idle();

        /* Getters */

        auto get_device() const -> ID3D12Device9*;
        auto get_frame_index() const -> u32;

        auto get_current_back_buffer(HWND hwnd) -> TextureResourceD3D12*;

    private:
        auto select_adapter(IDXGIFactory7* factory) -> IDXGIAdapter4*;
        auto determine_max_feature_level(IDXGIAdapter* adapter) -> D3D_FEATURE_LEVEL;

        struct Surface;
        auto get_surface_from_hwnd(HWND hwnd) -> Surface*;

        bool create_swapchain(Surface& surface);
        void destroy_swapchain(Surface& surface);

    private:
        ComPtr<ID3D12Device9> m_device{ nullptr };
        DWORD m_debugCallbackCookie{};

        Owned<QueueD3D12> m_graphicsQueue{ nullptr };
        u32 m_frameIndex{};

        std::array<u64, g_FrameBufferCount> m_endOfFrameFenceValues{};

        ComPtr<IDXGIFactory7> m_factory{ nullptr };
        struct Surface
        {
            sizet index{};
            HWND hwnd{};
            i32 width{};
            i32 height{};
            ComPtr<IDXGISwapChain4> swapchain{ nullptr };
            std::array<Owned<TextureResourceD3D12>, g_FrameBufferCount> renderTargets{};
        };
        std::vector<Surface> m_surfaces{};

        ComPtr<ID3D12DescriptorHeap> m_rtvHeap{ nullptr };
        u32 m_rtvDescriptorSize{};
    };
}