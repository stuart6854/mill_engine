#include "d3d12_device.hpp"

#include "mill/core/debug.hpp"
#include "d3d12_core.hpp"
#include "d3d12_context.hpp"
#include "d3d12_resources.hpp"
#include <memory>

namespace mill::platform
{
    constexpr D3D_FEATURE_LEVEL minimum_feature_level = D3D_FEATURE_LEVEL_11_0;

    void d3d12_debug_callback(
        D3D12_MESSAGE_CATEGORY /*Category*/, D3D12_MESSAGE_SEVERITY Severity, D3D12_MESSAGE_ID ID, LPCSTR pDescription, void* /*pContext*/)
    {
        if (Severity == D3D12_MESSAGE_SEVERITY_ERROR || Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION)
        {
            LOG_ERROR("D3D12 - [{}] {}", ID, pDescription);
        }
        else if (Severity == D3D12_MESSAGE_SEVERITY_WARNING)
        {
            LOG_WARN("D3D12 - [{}] {}", ID, pDescription);
        }
        else
        {
            LOG_DEBUG("D3D12 - [{}] {}", ID, pDescription);
        }
    }

    bool DeviceD3D12::inititialise()
    {
        HRESULT hr{ S_OK };
        u32 factory_flags{ 0 };

#ifdef MILL_DEBUG
        // Enable debugging layer. Requires "Graphics Tools" optional feature
        {
            ComPtr<ID3D12Debug6> debug_interface = nullptr;
            hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface));
            if (FAILED(hr))
            {
                LOG_ERROR("RendererDX12 - Failed to get debug interface!");
            }
            debug_interface->EnableDebugLayer();
            // debug_interface->SetEnableGPUBasedValidation();
        }

        // Enable debug errors/warnings
        factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
        hr = CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&m_factory));
        if (FAILED(hr))
        {
            LOG_ERROR("DeviceD3D12 - Failed to create DXGI Factory!");
            return false;
        }

        // Decide which adapter (GPU) to use, if any
        ComPtr<IDXGIAdapter4> adapter{ nullptr };
        adapter.Attach(select_adapter(m_factory.Get()));
        if (adapter == nullptr)
        {
            LOG_ERROR("DeviceD3D12 - Failed to select an adapter!");
            return false;
        }

        // Determine the maximum supported feature level
        auto max_feature_level = determine_max_feature_level(adapter.Get());
        ASSERT(max_feature_level >= minimum_feature_level);

        // Create device (virtual adapter)
        hr = D3D12CreateDevice(adapter.Get(), max_feature_level, IID_PPV_ARGS(&m_device));
        if (FAILED(hr))
        {
            LOG_ERROR("DeviceD3D12 - Failed to create device!");
            return false;
        }

#ifdef MILL_DEBUG
        {
            // Enable debug breaks & callback
            ComPtr<ID3D12InfoQueue1> info_queue = nullptr;
            m_device->QueryInterface(IID_PPV_ARGS(&info_queue));
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

            D3D12_MESSAGE_ID hide[] = { D3D12_MESSAGE_ID_CREATE_COMMANDLIST12, D3D12_MESSAGE_ID_DESTROY_COMMANDLIST12 };
            D3D12_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            info_queue->AddStorageFilterEntries(&filter);

            assert_if_failed(info_queue->RegisterMessageCallback(
                &d3d12_debug_callback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_debugCallbackCookie));
        }
#endif

        D3D12_COMMAND_QUEUE_DESC cmd_queue_desc{};
        cmd_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        hr = m_device->CreateCommandQueue(&cmd_queue_desc, IID_PPV_ARGS(&m_cmdQueue));
        if (FAILED(hr))
        {
            LOG_ERROR("DeviceD3D12 - Failed to create command queue!");
            return false;
        }

        assert_if_failed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue = 0;

        m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
        ASSERT(m_fenceEvent != nullptr);

        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{};
        rtv_heap_desc.NumDescriptors = g_FrameBufferCount;
        rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        assert_if_failed(m_device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        return true;
    }

    bool DeviceD3D12::shutdown()
    {
        wait_for_idle();

        CloseHandle(m_fenceEvent);
        m_fence = nullptr;

        for (auto& surface : m_surfaces)
        {
            destroy_swapchain(surface);
        }
        m_surfaces.clear();

        m_rtvHeap = nullptr;

        m_cmdQueue = nullptr;
        m_factory = nullptr;

#ifdef MILL_DEBUG
        {
            // Disable debug message breaks & callback
            ComPtr<ID3D12InfoQueue1> info_queue = nullptr;
            m_device->QueryInterface(IID_PPV_ARGS(&info_queue));
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
            info_queue->UnregisterMessageCallback(m_debugCallbackCookie);
        }
        {
            // Report any live objects
            ComPtr<ID3D12DebugDevice2> debug_device{ nullptr };
            m_device->QueryInterface(IID_PPV_ARGS(&debug_device));
            m_device = nullptr;
            // #NOTE: This will always report the debug_device
            debug_device->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
        }
#endif

        return true;
    }

    void DeviceD3D12::begin_frame()
    {
        ASSERT(m_device);

        m_frameIndex = (m_frameIndex + 1) % g_FrameBufferCount;
    }

    void DeviceD3D12::end_frame()
    {
        ASSERT(m_device);

        const auto signalValue = m_fenceValue;
        m_cmdQueue->Signal(m_fence.Get(), signalValue);
        ++m_fenceValue;

        // Wait until the previous frame is finished
        if (m_fence->GetCompletedValue() < signalValue)
        {
            assert_if_failed(m_fence->SetEventOnCompletion(signalValue, m_fenceEvent));
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
    }

    void DeviceD3D12::present()
    {
        ASSERT(m_device);

        for (auto& surface : m_surfaces)
        {
            surface.swapchain->Present(1, 0);
        }
    }

    bool DeviceD3D12::add_surface(HWND hwnd, i32 width, i32 height)
    {
        ASSERT(m_device);

        if (get_surface_from_hwnd(hwnd) != nullptr)
        {
            LOG_WARN("DeviceD3D12 - Cannot create a surface for HWND ({}) when it already exists.", fmt::ptr(hwnd));
            return false;
        }

        auto& surface = m_surfaces.emplace_back();
        surface.index = m_surfaces.size() - 1;
        surface.hwnd = hwnd;
        surface.width = width;
        surface.height = height;
        if (!create_swapchain(surface))
        {
            LOG_ERROR("DeviceD3D12 - Failed to add surface for HWND ({})!", fmt::ptr(hwnd));
            return false;
        }

        return true;
    }

    void DeviceD3D12::remove_surface(HWND hwnd)
    {
        ASSERT(m_device);

        auto* surface = get_surface_from_hwnd(hwnd);
        if (surface == nullptr)
        {
            LOG_WARN("DeviceD3D12 -  Cannot destroy a surface that does not exist for HWND ({}).", fmt::ptr(hwnd));
            return;
        }
        const auto index = surface->index;
        destroy_swapchain(*surface);
        m_surfaces.erase(m_surfaces.begin() + index);
    }

    auto DeviceD3D12::create_context() -> Owned<ContextD3D12>
    {
        ASSERT(m_device);

        return CreateOwned<ContextD3D12>(*this, D3D12_COMMAND_LIST_TYPE_DIRECT);
    }

    void DeviceD3D12::destroy_context(Owned<ContextD3D12> context)
    {
        context = nullptr;
    }

    void DeviceD3D12::submit_context(ContextD3D12& context)
    {
        context.get_cmd_list()->Close();

        ID3D12CommandList* cmdLists[] = { context.get_cmd_list() };
        m_cmdQueue->ExecuteCommandLists(_countof(cmdLists), &cmdLists[0]);

        // Receipt
        // Reciept.frameId = m_frameIndex
        // receipt.submissionIndex = m_contextSubmissions.
    }

    void DeviceD3D12::wait_on_context(/*Some submission receipt*/) {}

    void DeviceD3D12::wait_for_idle()
    {
        // Signal and increment fence value
        const u64 signalValue = m_fenceValue;
        assert_if_failed(m_cmdQueue->Signal(m_fence.Get(), signalValue));
        ++m_fenceValue;

        // Wait until the previous frame is finished
        if (m_fence->GetCompletedValue() < signalValue)
        {
            assert_if_failed(m_fence->SetEventOnCompletion(signalValue, m_fenceEvent));
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
    }

    auto DeviceD3D12::get_device() const -> ID3D12Device10*
    {
        return m_device.Get();
    }

    auto DeviceD3D12::get_frame_index() const -> u32
    {
        return m_frameIndex;
    }

    auto DeviceD3D12::get_current_back_buffer(HWND hwnd) -> TextureResourceD3D12*
    {
        auto* surface = get_surface_from_hwnd(hwnd);
        if (surface == nullptr)
        {
            LOG_ERROR("DeviceD3D12 - Could not get current back buffer for unknown surface ({})!", fmt::ptr(hwnd));
            return nullptr;
        }

        const auto backbufferIndex = surface->swapchain->GetCurrentBackBufferIndex();
        return surface->renderTargets[backbufferIndex].get();
    }

    auto DeviceD3D12::select_adapter(IDXGIFactory7* factory) -> IDXGIAdapter4*
    {
        IDXGIAdapter4* adapter{ nullptr };

        // Get adapters in descending order of performance
        for (u32 i = 0;
             factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
             ++i)
        {
            // Pick the first adapter that supports the minimum feature level
            if (SUCCEEDED(D3D12CreateDevice(adapter, minimum_feature_level, __uuidof(ID3D12Device), nullptr)))
            {
                return adapter;
            }

            safe_release(adapter);
        }

        return nullptr;
    }

    auto DeviceD3D12::determine_max_feature_level(IDXGIAdapter* adapter) -> D3D_FEATURE_LEVEL
    {
        constexpr D3D_FEATURE_LEVEL feature_levels[]{
            D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_2,
        };

        D3D12_FEATURE_DATA_FEATURE_LEVELS feature_levels_info{};
        feature_levels_info.NumFeatureLevels = _countof(feature_levels);
        feature_levels_info.pFeatureLevelsRequested = feature_levels;

        ComPtr<ID3D12Device> device{ nullptr };
        D3D12CreateDevice(adapter, minimum_feature_level, IID_PPV_ARGS(&device));
        device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &feature_levels_info, sizeof(feature_levels_info));
        return feature_levels_info.MaxSupportedFeatureLevel;
    }

    auto DeviceD3D12::get_surface_from_hwnd(HWND hwnd) -> Surface*
    {
        for (auto& surface : m_surfaces)
        {
            if (surface.hwnd == hwnd)
            {
                return &surface;
            }
        }

        return nullptr;
    }

    bool DeviceD3D12::create_swapchain(Surface& surface)
    {
        DXGI_SWAP_CHAIN_DESC1 swapchain_desc{};
        swapchain_desc.Width = 0;   // surface.width;
        swapchain_desc.Height = 0;  // surface.height;
        swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapchain_desc.SampleDesc.Count = 1;
        swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_desc.BufferCount = g_FrameBufferCount;
        swapchain_desc.Scaling = DXGI_SCALING_STRETCH;
        swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        ComPtr<IDXGISwapChain1> swapchain = nullptr;

        auto hr = m_factory->CreateSwapChainForHwnd(m_cmdQueue.Get(), surface.hwnd, &swapchain_desc, nullptr, nullptr, &swapchain);
        if (FAILED(hr))
        {
            LOG_ERROR("DeviceD3D12 - Failed to create swapchain for hwnd ({})!", fmt::ptr(surface.hwnd));
            return false;
        }

        assert_if_failed(swapchain.As(&surface.swapchain));

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (u32 i = 0; i < g_FrameBufferCount; ++i)
        {
            ID3D12Resource* backbufferResource{ nullptr };
            DescriptorD3D12 backbufferRTVHandle{};
            backbufferRTVHandle.heapIndex = i;
            backbufferRTVHandle.cpuHandle = rtvHandle;
            rtvHandle.Offset(1, m_rtvDescriptorSize);

            assert_if_failed(surface.swapchain->GetBuffer(i, IID_PPV_ARGS(&backbufferResource)));
            m_device->CreateRenderTargetView(backbufferResource, nullptr, backbufferRTVHandle.cpuHandle);

            auto& texture_resource = surface.renderTargets[i];
            texture_resource = std::make_unique<TextureResourceD3D12>();
            texture_resource->resource = backbufferResource;
            texture_resource->rtvDescriptor = backbufferRTVHandle;
        }

        return true;
    }

    void DeviceD3D12::destroy_swapchain(Surface& surface)
    {
        for (auto& render_target : surface.renderTargets)
        {
            render_target->resource->Release();
            render_target = {};
        }
        surface.swapchain = nullptr;
    }

}
